#include <errno.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "specs.h"
#include "storage.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"
#include "slist.h"
#include "bitmap.h"

static inode_t *root_nodep;

int inode_for_path_comps_in(int dinum, slist_t *comps)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));

  // Get the node.
  inode_t *dnodep = get_inode(dinum);

  assert(dnodep->mode & INODE_DIR);

  // If the components are null return no such file error.
  if (!comps)
  {
    return -ENOENT;
  }

  // Ensure the data is not null.
  assert(comps->data);

  // If the string is empty and the next token exists, i.e. double slash, ignore it and move on to
  // the next token. But, if the string is empty and there is no next token, return the parent dir.
  if (!strcmp(comps->data, ""))
  {
    if (comps->next)
    {
      return inode_for_path_comps_in(dinum, comps->next);
    }

    return dinum;
  }

  int entry_inum;

  // Lookup the inode in the directory. If it is an error, we can conveniently return this error
  // code. If the lookup succeeded and this is the end of the path, return the entry.
  if ((entry_inum = directory_lookup(dnodep, comps->data)) < 0 || !comps->next)
  {
    return entry_inum;
  }

  // We ensure the entry node is a directory since there is more path coming.
  if (!(get_inode(entry_inum)->mode & INODE_DIR))
  {
    return -ENOTDIR;
  }

  // Continue on down the path looking in that directory.
  return inode_for_path_comps_in(entry_inum, comps->next);
}

int inode_for_path_in(int dinum, const char *path)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));
  assert(get_inode(dinum)->mode & INODE_DIR);
  assert(path);

  // Split the path string into components and delegate to the list version of this function.
  slist_t *path_components = slist_explode(path, '/');

  printf("SIZE: %d\n", slist_size(path_components));
  printf("[");
  slist_print(path_components, ", ");
  printf("]\n");

  int inum = inode_for_path_comps_in(dinum, path_components);
  slist_free(path_components);

  return inum;
}

int inode_for_path(const char *path)
{
  assert(path);

  // Start from the root.
  return inode_for_path_in(ROOT_INUM, path);
}

void storage_init(const char *path)
{
  assert(path);

  // Create a memory map and initialize the disk blocks and.
  blocks_init(path);

  printf("\033[0;1;31mREMEMBER TO REMOVE THIS CLEAR FUNCTION DUMBASS!\033[0m\n");
  blocks_clear();

  // Allocate the root inode and make it a directory if it doesn't already exist. Note that this
  // relies on ROOT_INUM being 0. Otherwise, there is no guarantee ROOT_INUM will be allocated.
  if (!bitmap_get(get_inode_bitmap(), ROOT_INUM))
  {
    assert(alloc_inode() == ROOT_INUM);
    directory_init(ROOT_INUM);
  }

  // Initialize a pointer to the root node structure.
  root_nodep = get_inode(ROOT_INUM);

  #define INODES 16

  int inums[INODES];
  inode_t *inodes[INODES];

  for (size_t i = 0; i < INODES; i++)
  {
    inums[i] = alloc_inode();
    inodes[i] = get_inode(inums[i]);
    
    if (i % 2 == 0)
    {
      directory_init(inums[i]);
    }
  }
  
  directory_put(ROOT_INUM, "code", inums[0], TRUE);
  directory_put(ROOT_INUM, "school stuff", inums[2], TRUE);
  directory_put(ROOT_INUM, "README.md", inums[1], TRUE);
  
  directory_put(inums[0], "main.c", inums[1], TRUE);
  directory_put(inums[0], "util.h", inums[3], TRUE);
  directory_put(inums[0], "util.c", inums[5], TRUE);
  directory_put(inums[0], "Makefile", inums[7], TRUE);

  directory_put(inums[2], "CS4100", inums[4], TRUE);
  directory_put(inums[2], "DS4400", inums[6], TRUE);
  directory_put(inums[2], "resume.pdf", inums[9], TRUE);

  directory_put(inums[6], "hw1.pdf", inums[11], TRUE);
  directory_put(inums[6], "hw2.pdf", inums[13], TRUE);
  directory_put(inums[6], "hw3.pdf", inums[15], TRUE);
  directory_put(inums[6], "01234567890123456789012345678901234567890123456789012345678this will all be truncated", inums[8], TRUE);

  printf("%d\n", inode_for_path("/////school stuff////////DS4400/////////01234567890123456789012345678901234567890123456789012345678"));
}

void storage_deinit(void)
{
  // Persist the blocks in the memory map to disk .
  blocks_free();
}

int storage_stat(const char *path, struct stat *st)
{
}

int storage_read(const char *path, char *buf, size_t size, off_t offset)
{
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset)
{
}

int storage_truncate(const char *path, off_t size)
{
}

int storage_mknod(const char *path, int mode)
{
}

int storage_unlink(const char *path)
{
}

int storage_link(const char *from, const char *to)
{
}

int storage_rename(const char *from, const char *to)
{
}

int storage_set_time(const char *path, const struct timespec ts[2])
{
}

slist_t *storage_list(const char *path)
{
}
