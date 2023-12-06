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
  assert(inode_exists(inum));

  inode_t *dnodep = inode_get(inum);
  dnodep->mode = dnodep->mode & ~INODE_FILE | INODE_DIR;

  directory_add_entry(inum, ".", inum, FALSE);
}

int directory_total_entry_count(inode_t *dnodep)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);

  // Determine how many entries are present (should be a clean divide).
  return inode_total_size(dnodep) / sizeof(dirent_t);
}

int directory_populated_entry_count(inode_t *dnodep)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);

  int count = 0;

  for (int entry_num = 0; entry_num < directory_total_entry_count(dnodep); entry_num++)
  {
    if (directory_get_entry(dnodep, entry_num)->inum >= 0)
    {
      count++;
    }
  }

  return count;
}

dirent_t *directory_get_entry(inode_t *dnodep, int entry_num)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(entry_num >= 0);
  assert(entry_num < directory_total_entry_count(dnodep));

  int entry_block = (sizeof(dirent_t) * entry_num) / BLOCK_SIZE;
  int entry_offset = (sizeof(dirent_t) * entry_num) % BLOCK_SIZE;
  int bnum = inode_get_bnum(dnodep, entry_block);

  return block_get(bnum) + entry_offset;
}

int directory_lookup_entry_num(inode_t *dnodep, const char *name)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(name);

  // If the name is too long immediately rule it out.
  if (strlen(name) > MAX_DIR_ENTRY_NAME_LEN - 1)
  {
    return -ENOENT;
  }

  dirent_t *entryp;

  // Search for an entry with the given name and return the entry number.
  for (int entry_num = 0; entry_num < directory_total_entry_count(dnodep); entry_num++)
  {
    entryp = directory_get_entry(dnodep, entry_num);

    if (entryp->inum >= 0 && !strcmp(name, entryp->name))
    {
      return entry_num;
    }
  }

  // Return -ENOENT to indicate if no directory entry was found.
  return -ENOENT;
}

int directory_lookup_inum(inode_t *dnodep, const char *name)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(name);

  int entry_num = directory_lookup_entry_num(dnodep, name);

  if  (entry_num < 0)
  {
    return entry_num;
  }

  return directory_get_entry(dnodep, entry_num)->inum;
}

int directory_rename_entry(inode_t *dnodep, int entry_num, const char *name)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(entry_num >= 0);
  assert(directory_get_entry(dnodep, entry_num)->inum >= 0);
  assert(name);

  // Ensure the name is not too long.
  if (strlen(name) >= MAX_DIR_ENTRY_NAME_LEN)
  {
    return -ENAMETOOLONG;
  }

  // Ensure an entry with the given name does not already exist.
  if (directory_lookup_entry_num(dnodep, name) >= 0)
  {
    return -EEXIST;
  }

  // Copy the name into the buffer and ensure a null terminator is included.
  dirent_t *entryp = directory_get_entry(dnodep, entry_num);
  strcpy(entryp->name, name);

  // Return the entry number.
  return entry_num;
}

int directory_add_entry(int dinum, const char *name, int entry_inum, bool_t update_child)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));
  
  inode_t *dnodep = inode_get(dinum);

  assert(dnodep->mode & INODE_DIR);
  assert(name);
  assert(entry_inum >= 0);
  assert(inode_exists(entry_inum));

  // Ensure the name is not too long.
  if (strlen(name) >= MAX_DIR_ENTRY_NAME_LEN)
  {
    return -ENAMETOOLONG;
  }
  
  dirent_t *entryp;
  int total_entry_count = directory_total_entry_count(dnodep);
  int entry_num;

  // Find an open entry if one exists.
  for (entry_num = 0; entry_num < total_entry_count; entry_num++)
  {
    entryp = directory_get_entry(dnodep, entry_num);

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
  if (entry_num >= total_entry_count)
  {
    // Since no open entry exists, create a new one and grow the inode. If the inode returns -ENOSPC
    // indicating that the disk is full, return -ENOSPC immediately.
    if (inode_grow(dnodep, sizeof(dirent_t)) < 0)
    {
      return -ENOSPC;
    }

    // Insert the proper data into the entry.
    entryp = directory_get_entry(dnodep, total_entry_count);
  }

  entryp->inum = entry_inum;
  directory_rename_entry(dnodep, entry_num, name);

  if (update_child)
  {
    inode_t *entry_nodep = inode_get(entry_inum);

    entry_nodep->refs += 1;

    // Put an entry for .. in the child.
    if (entry_nodep->mode & INODE_DIR)
    {
      directory_add_entry(entry_inum, "..", dinum, FALSE);
    }
  }

  // Return the directory entry index.
  return entry_num;
}

int directory_delete(inode_t *dnodep, const char *name, bool_t update_child)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(name);

  dirent_t *entryp;
  inode_t *entry_nodep;

  for (int entry_num = 0; entry_num < directory_total_entry_count(dnodep); entry_num++)
  {
    entryp = directory_get_entry(dnodep, entry_num);

    if (!strcmp(name, entryp->name))
    {
      entry_nodep = inode_get(entryp->inum);

      entryp->inum = -1;

      if (update_child)
      {
        entry_nodep->refs -= 1;

        if (entry_nodep->mode & INODE_DIR)
        {
          directory_delete(entry_nodep, "..", FALSE);
        }
      }

      return entry_num;
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

  for (int entry_num = directory_total_entry_count(dnodep) - 1; entry_num >= 0; entry_num--)
  {
    name = directory_get_entry(dnodep, entry_num)->name;
    list = slist_cons(name, list);
  }

  return list;
}

void directory_print(inode_t *dnodep, bool_t include_empty_entries)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);

  dirent_t *entryp;

  printf("\033[0;1mIdx\tiNum\tName\033[0m\n");

  for (int entry_num = 0; entry_num < directory_total_entry_count(dnodep); entry_num++)
  {
    entryp = directory_get_entry(dnodep, entry_num);

    if (include_empty_entries || entryp->inum >= 0)
    {
      printf("%d\t%d\t%s\n", entry_num, entryp->inum, entryp->name);
    }
  }
}

void directory_print_leveled_tree(inode_t *dnodep, int level)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);
  assert(level >= 0);

  dirent_t *entryp;
  inode_t *subnodep;

  for (int entry_num = 0; entry_num < directory_total_entry_count(dnodep); entry_num++)
  {
    entryp = directory_get_entry(dnodep, entry_num);

    if (entryp->inum < 0 || !strcmp(entryp->name, ".") || !strcmp(entryp->name, ".."))
    {
      continue;
    }

    repeat_print("  ", level);
    printf("%s\n", entryp->name);

    subnodep = inode_get(entryp->inum);

    if (subnodep->mode & INODE_DIR)
    {
      directory_print_leveled_tree(subnodep, level + 1);
    }
  }
}

void directory_print_tree(inode_t *dnodep)
{
  assert(dnodep);
  assert(dnodep->mode & INODE_DIR);

  // Call the recursive helper.
  directory_print_leveled_tree(dnodep, 0);
}
