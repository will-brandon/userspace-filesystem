#include <errno.h>
#include <assert.h>
#include <string.h>
#include "specs.h"
#include "bitmap.h"
#include "inode.h"
#include "directory.h"

void directory_init(int inum)
{
  assert(inum >= 0);
  assert(bitmap_get(get_inode_bitmap(), inum));

  inode_t *dnodep = get_inode(inum);
  dnodep->mode = dnodep->mode & ~INODE_FILE | INODE_DIR;

  directory_put(inum, ".", inum, FALSE);
}

int directory_entry_count(inode_t *dnodep)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);

  // Determine how many entries are present (should be a clean divide).
  return inode_total_size(dnodep) / sizeof(dirent_t);
}

dirent_t *directory_get(inode_t *dnodep, int i)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(i >= 0);
  assert(i < directory_entry_count(dnodep));

  int entry_block = (sizeof(dirent_t) * i) / BLOCK_SIZE;
  int entry_offset = (sizeof(dirent_t) * i) % BLOCK_SIZE;
  int bnum = inode_get_bnum(dnodep, entry_block);

  return blocks_get_block(bnum) + entry_offset;
}

size_t directory_rename(dirent_t *entryp, const char *name)
{
  assert(entryp);
  assert(entryp->inum >= 0);
  assert(name);

  // Get the length of the name string and determine how much of it will actually fit.
  size_t name_len = strlen(name);
  size_t copy_size = MIN(name_len, DIR_NAME_LENGTH - 1);

  // Copy the name into the buffer and ensure a null terminator is included.
  memcpy(entryp->name, name, copy_size);
  entryp->name[copy_size] = '\0';

  // Return how much of the name was actually used.
  return copy_size;
}

int directory_lookup(inode_t *dnodep, const char *name)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(name);

  dirent_t *entryp;

  for (int i = 0; i < directory_entry_count(dnodep); i++)
  {
    entryp = directory_get(dnodep, i);

    if (entryp->inum >= 0 && !strcmp(name, entryp->name))
    {
      return entryp->inum;
    }
  }

  // Return -ENOENT to indicate if no directory entry was found.
  return -ENOENT;
}

// Check for same name existing
int directory_put(int dinum, const char *name, int entry_inum, bool_t update_child)
{
  assert(dinum >= 0);
  assert(bitmap_get(get_inode_bitmap(), dinum));
  
  inode_t *dnodep = get_inode(dinum);

  assert(dnodep->mode & INODE_DIR);
  assert(name);
  assert(entry_inum >= 0);
  assert(bitmap_get(get_inode_bitmap(), entry_inum));
  
  dirent_t *entryp;
  int entry_count = directory_entry_count(dnodep);
  int i;

  // Find an open entry if one exists.
  for (i = 0; i < entry_count; i++)
  {
    entryp = directory_get(dnodep, i);

    // Indicate that a file with the given name already exists.
    if (!strcmp(name, entryp->name))
    {
      return -EEXIST;
    }

    if (entryp->inum < 0)
    {
      break;
    }
  }

  // If we didn't find an empty entry, create a new one.
  if (i >= entry_count)
  {
    // Since no open entry exists, create a new one and grow the inode. If the inode returns -ENOSPC
    // indicating that the disk is full, return -ENOSPC immediately.
    if (grow_inode(dnodep, sizeof(dirent_t)) < 0)
    {
      return -ENOSPC;
    }

    // Insert the proper data into the entry.
    entryp = directory_get(dnodep, entry_count);
  }

  entryp->inum = entry_inum;
  directory_rename(entryp, name);

  if (update_child)
  {
    inode_t *entry_nodep = get_inode(entry_inum);

    entry_nodep->refs += 1;

    // Put an entry for .. in the child.
    if (entry_nodep->mode & INODE_DIR)
    {
      directory_put(entry_inum, "..", dinum, FALSE);
    }
  }

  // Return the directory entry index.
  return entry_count;
}

int directory_delete(inode_t *dnodep, const char *name, bool_t update_child)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(name);

  dirent_t *entryp;
  inode_t *entry_nodep;

  for (int i = 0; i < directory_entry_count(dnodep); i++)
  {
    entryp = directory_get(dnodep, i);

    if (!strcmp(name, entryp->name))
    {
      entry_nodep = get_inode(entryp->inum);

      entryp->inum = -1;

      if (update_child)
      {
        entry_nodep->refs -= 1;

        if (entry_nodep->mode & INODE_DIR)
        {
          directory_delete(entry_nodep, "..", FALSE);
        }
      }

      return i;
    }
  }

  return -ENOENT;
}

slist_t *directory_list(inode_t *dnodep)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);

  slist_t *list = NULL;
  const char *name;

  for (int i = directory_entry_count(dnodep) - 1; i >= 0; i--)
  {
    name = directory_get(dnodep, i)->name;
    list = slist_cons(name, list);
  }

  return list;
}

void print_directory(inode_t *dnodep, bool_t include_empty_entries)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);

  dirent_t *entryp;

  printf("\033[0;1mIdx\tiNum\tName\033[0m\n");

  for (int i = 0; i < directory_entry_count(dnodep); i++)
  {
    entryp = directory_get(dnodep, i);

    if (include_empty_entries || entryp->inum >= 0)
    {
      printf("%d\t%d\t%s\n", i, entryp->inum, entryp->name);
    }
  }
}
