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

  inode_t *nodep = get_inode(inum);
  nodep->mode = nodep->mode & ~INODE_FILE | INODE_DIR;

  directory_put(nodep, ".", inum);
}

int directory_entry_count(inode_t *nodep)
{
  assert(nodep);
  assert(nodep->mode & INODE_DIR);

  // Determine how many entries are present (should be a clean divide).
  return inode_total_size(nodep) / sizeof(dirent_t);
}

dirent_t *directory_get(inode_t *nodep, int i)
{
  assert(nodep);
  assert(nodep->mode & INODE_DIR);
  assert(i >= 0);
  assert(i < directory_entry_count(nodep));

  int entry_block = (sizeof(dirent_t) * i) / BLOCK_SIZE;
  int entry_offset = (sizeof(dirent_t) * i) % BLOCK_SIZE;
  int bnum = inode_get_bnum(nodep, entry_block);

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
  memcpy(entryp->name, name, name_len);
  entryp->name[name_len] = '\0';

  // Return how much of the name was actually used.
  return copy_size;
}

int directory_lookup(inode_t *nodep, const char *name)
{
  assert(nodep);
  assert(nodep->mode & INODE_DIR);
  assert(name);

  dirent_t *entryp;

  for (int i = 0; i < directory_entry_count(nodep); i++)
  {
    entryp = directory_get(nodep, i);

    if (entryp->inum >= 0 && !strcmp(name, entryp->name))
    {
      return entryp->inum;
    }
  }

  // Return -1 to indicate if no directory entry was found.
  return -1;
}

int directory_lookup_path(inode_t *nodep, const char *path)
{
  assert(nodep);
  assert(nodep->mode & INODE_DIR);
  assert(path);
}

// Check for same name existing
int directory_put(inode_t *nodep, const char *name, int inum)
{
  assert(nodep);
  assert(nodep->mode & INODE_DIR);
  assert(name);
  assert(inum >= 0);
  assert(bitmap_get(get_inode_bitmap(), inum));
  
  dirent_t *entryp;
  int entry_count = directory_entry_count(nodep);

  // Find an open entry if one exists.
  for (int i = 0; i < entry_count; i++)
  {
    entryp = directory_get(nodep, i);

    // Indicate that a file with the given name already exists.
    if (!strcmp(name, entryp->name))
    {
      return -EEXIST;
    }

    if (entryp->inum < 0)
    {
      entryp->inum = inum;
      directory_rename(entryp, name);
      get_inode(inum)->refs += 1;

      return i;
    }
  }

  // Since no open entry exists, create a new one and grow the inode. If the inode returns -ENOSPC
  // indicating that the disk is full, return -ENOSPC immediately.
  if (grow_inode(nodep, sizeof(dirent_t)) < 0)
  {
    return -ENOSPC;
  }

  // Insert the proper data into the entry.
  entryp = directory_get(nodep, entry_count);
  entryp->inum = inum;
  directory_rename(entryp, name);
  get_inode(inum)->refs += 1;

  // Return the directory entry index.
  return entry_count;
}

int directory_delete(inode_t *nodep, const char *name)
{
  assert(nodep);
  assert(nodep->mode & INODE_DIR);
  assert(name);

  dirent_t *entryp;

  for (int i = 0; i < directory_entry_count(nodep); i++)
  {
    entryp = directory_get(nodep, i);

    if (!strcmp(name, entryp->name))
    {
      entryp->inum = -1;
      get_inode(entryp->inum)->refs -= 1;

      return i;
    }
  }

  return -ENOENT;
}

slist_t *directory_list(const char *path)
{
  assert(path);
}

void print_directory(inode_t *nodep, bool_t include_empty_entries)
{
  assert(nodep);
  assert(nodep->mode & INODE_DIR);

  dirent_t *entryp;

  printf("\033[0;1mIdx\tiNum\tName\033[0m\n");

  for (int i = 0; i < directory_entry_count(nodep); i++)
  {
    entryp = directory_get(nodep, i);

    if (include_empty_entries || entryp->inum >= 0)
    {
      printf("%d\t%d\t%s\n", i, entryp->inum, entryp->name);
    }
  }
}
