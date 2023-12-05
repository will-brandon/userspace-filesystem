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

  directory_put(nodep, ".", inum);
}

int directory_entry_count(inode_t *nodep)
{
  assert(nodep);

  // Determine how many entries are present (should be a clean divide).
  return inode_total_size(nodep) / sizeof(dirent_t);
}

dirent_t *directory_get(inode_t *nodep, int i)
{
  int file_bnum = (sizeof(dirent_t) * i) / BLOCK_SIZE;
  int bnum = inode_get_bnum(nodep, file_bnum);
  int offset = (sizeof(dirent_t) * i) % BLOCK_SIZE;

  return blocks_get_block(bnum) + offset;
}

int directory_lookup(inode_t *nodep, const char *name)
{
  assert(nodep);
  assert(name);

  dirent_t *entryp;

  for (int i = 0; i < directory_entry_count(nodep); i++)
  {
    entryp = directory_get(nodep, i);

    if (entryp->inum >= 0 && !strcmp(entryp->name, name))
    {
      return entryp->inum;
    }
  }
}

int directory_lookup_path(inode_t *nodep, const char *path)
{
  assert(nodep);
  assert(path);
}

// Check for same name existing
int directory_put(inode_t *nodep, const char *name, int inum)
{
}

int directory_delete(inode_t *nodep, const char *name)
{
}

slist_t *directory_list(const char *path)
{
}

void print_directory(inode_t *nodep)
{
  assert(nodep);

  dirent_t *entryp;

  printf("#\tiNum\tName");

  for (int i = 0; i < directory_entry_count(nodep); i++)
  {
    entryp = directory_get(nodep, i);

    printf("%d\t%d\t%s\n", i, entryp->inum, entryp->name);
  }
}
