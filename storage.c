#define _GNU_SOURCE

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "specs.h"
#include "storage.h"
#include "block.h"
#include "inode.h"
#include "directory.h"
#include "slist.h"
#include "bitmap.h"

#define ROOT_INUM 0

static inode_t *root_nodep;

void test(void)
{
  #define INODES 16

  int inums[INODES];
  inode_t *inodes[INODES];

  for (size_t i = 0; i < INODES; i++)
  {
    inums[i] = inode_alloc();
    inodes[i] = inode_get(inums[i]);
    
    if (i % 2 == 0)
    {
      directory_init(inums[i]);
    }
  }
  
  directory_add_entry(ROOT_INUM, "code", inums[0], TRUE);
  directory_add_entry(ROOT_INUM, "school stuff", inums[2], TRUE);
  directory_add_entry(ROOT_INUM, "README.md", inums[1], TRUE);
  
  directory_add_entry(inums[0], "main.c", inums[1], TRUE);
  directory_add_entry(inums[0], "util.h", inums[3], TRUE);
  directory_add_entry(inums[0], "util.c", inums[5], TRUE);
  directory_add_entry(inums[0], "Makefile", inums[7], TRUE);

  directory_add_entry(inums[2], "CS4100", inums[4], TRUE);
  directory_add_entry(inums[2], "DS4400", inums[6], TRUE);
  directory_add_entry(inums[2], "resume.pdf", inums[9], TRUE);

  directory_add_entry(inums[6], "hw1.pdf", inums[11], TRUE);
  directory_add_entry(inums[6], "hw2.pdf", inums[13], TRUE);
  directory_add_entry(inums[6], "hw3.pdf", inums[15], TRUE);
  directory_add_entry(inums[6], "01234567890123456789012345678901234567890123456789012345678this will all be truncated", inums[8], TRUE);
  
  /*
  int new_file_count = 15;

  printf("Creating %d new files in root.\n", new_file_count);

  char buffer[] = "01234567890123456789012345678901234567890123456789012345678901234567890123456789";

  for (int i = 0; i < new_file_count; i++)
  {
    int inum = alloc_inode();
    directory_add_entry(ROOT_INUM, buffer + i, inum, TRUE);
  }

  directory_print(root_nodep, TRUE);*/
}

int inum_for_path_comps_in(int dinum, slist_t *comps)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));

  // Get the node.
  inode_t *dnodep = inode_get(dinum);

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
      return inum_for_path_comps_in(dinum, comps->next);
    }

    return dinum;
  }

  int entry_inum;

  // Lookup the inode in the directory. If it is an error, we can conveniently return this error
  // code. If the lookup succeeded and this is the end of the path, return the entry.
  if ((entry_inum = directory_lookup_inum(dnodep, comps->data)) < 0 || !comps->next)
  {
    return entry_inum;
  }

  // We ensure the entry node is a directory since there is more path coming.
  if (!(inode_get(entry_inum)->mode & INODE_DIR))
  {
    return -ENOTDIR;
  }

  // Continue on down the path looking in that directory.
  return inum_for_path_comps_in(entry_inum, comps->next);
}

int inum_for_path_in(int dinum, const char *path)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));
  assert(inode_get(dinum)->mode & INODE_DIR);
  assert(path);

  // Split the path string into components and delegate to the list version of this function.
  slist_t *path_components = slist_explode(path, '/');
  int inum = inum_for_path_comps_in(dinum, path_components);
  slist_free(path_components);

  return inum;
}

int inum_for_path(const char *path)
{
  assert(path);

  // Start from the root.
  return inum_for_path_in(ROOT_INUM, path);
}

void storage_init(const char *path)
{
  assert(path);

  // Create a memory map and initialize the disk blocks and.
  block_init(path);

  printf("\033[0;1;31mREMEMBER TO REMOVE THIS CLEAR FUNCTION DUMBASS!\033[0m\n");
  storage_clear();

  // Allocate the root inode and make it a directory if it doesn't already exist. Note that this
  // relies on ROOT_INUM being 0. Otherwise, there is no guarantee ROOT_INUM will be allocated.
  if (!inode_exists(ROOT_INUM))
  {
    assert(inode_alloc() == ROOT_INUM);
    directory_init(ROOT_INUM);
  }

  // Initialize a pointer to the root node structure.
  root_nodep = inode_get(ROOT_INUM);

  test();
}

void storage_deinit(void)
{
  // Persist the blocks in the memory map to disk .
  block_deinit();
}

void storage_clear(void)
{
  // Clear all the blocks and reset the reserved blocks.
  block_clear();
}

int storage_access(const char *path, int mode)
{
  assert(path);

  // If none of the modes were requested, return a success code of 0.
  if (!(mode & F_OK & R_OK & W_OK & X_OK))
  {
    return 0;
  }

  // Lookup the inode inum at the given path.
  int inum = inum_for_path(path);

  // If a lookup error occured return the error code.
  if (inum < 0)
  {
    return inum;
  }

  // If only F_OK was tested, we're safe to return 0 here.
  if (!(mode & R_OK & W_OK & X_OK))
  {
    return 0;
  }

  // This is where the functionality could be extended to support R_OK, W_OK, and X_OK. For now,
  // we assume that all are permitted.
  return 0;
}

int storage_stat(const char *path, struct stat *stp)
{
  assert(path);
  assert(stp);

  // Lookup the inode inum at the given path.
  int inum = inum_for_path(path);

  // If a lookup error occured return the error code.
  if (inum < 0)
  {
    return inum;
  }

  // Get a pointer to the inode.
  inode_t *nodep = inode_get(inum);

  // Set all used stats.
  stp->st_ino = inum;
  stp->st_blksize = BLOCK_SIZE;
  stp->st_size = inode_total_size(nodep);
  stp->st_blocks = bytes_to_blocks(stp->st_size);
  stp->st_mode = nodep->mode;
  stp->st_nlink = nodep->refs;
  
  // Unused stats set to default.
  stp->st_dev = 0;
  stp->st_rdev = 0;
  stp->st_gid = 0;
  stp->st_uid = 0;
  stp->st_ctim.tv_sec = 0;
  stp->st_ctim.tv_nsec = 0;
  stp->st_mtim.tv_sec = 0;
  stp->st_mtim.tv_nsec = 0;
  stp->st_atim.tv_sec = 0;
  stp->st_atim.tv_nsec = 0;

  // Return 0 indicating no error occured.
  return 0;
}

int storage_mknod(const char *path, int mode)
{
  assert(path);

  // Allocate a new node.
  int inum = inode_alloc();

  // If an error occured allocating the node, return the error.
  if (inum < 0)
  {
    return inum;
  }

  // Set the mode of the node and return the success code 0.
  inode_get(inum)->mode = mode;
  return 0;
}

int storage_link(const char *from, const char *to)
{
  assert(from);
  assert(to);

  // Get the inum of the inode at the "from" path.
  int inum = inum_for_path(from);

  // Ensure the inode at the "from" path exists.
  if (inum < 0)
  {
    return -ENOENT;
  }

  // Ensure the inode at the "to" path does not already exist.
  if (inum_for_path(to) >= 0)
  {
    return -EEXIST;
  }

  
}

int storage_unlink(const char *path)
{
  assert(path);
}

int storage_rename(const char *from, const char *to)
{
}


int storage_truncate(const char *path, off_t size)
{
  assert(path);
  assert(size >= 0);

  // Lookup the inode inum at the given path.
  int inum = inum_for_path(path);

  // If a lookup error occured return the error code.
  if (inum < 0)
  {
    return inum;
  }

  // Get a pointer to the inode and determine its current size.
  inode_t *nodep = inode_get(inum);
  int size_delta = size - inode_total_size(nodep);
  int rv;

  // Grow the inode if the delta > 0.
  if (size_delta > 0)
  {
    rv = inode_grow(nodep, size_delta);
  }
  // Shrink the inode if the delta < 0.
  else if (size_delta < 0)
  {
    rv = inode_shrink(nodep, -size_delta);
  }

  // If an error occured return the error code. Otherise return 0 indicating success.
  return rv < 0 ? rv : 0;
}

int storage_read(const char *path, char *buf, size_t size, off_t offset)
{
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset)
{
}

int storage_list(const char *dpath, slist_t **namesp)
{
  assert(dpath);

  // Lookup the inode at the given path.
  int inum = inum_for_path(dpath);

  // If a lookup error occured return the error code.
  if (inum < 0)
  {
    return inum;
  }

  // Get a pointer to the inode.
  inode_t *dnodep = inode_get(inum);

  // If the node is not a directory return an error code.
  if (!(dnodep->mode & INODE_DIR))
  {
    return -ENOTDIR;
  }

  // Get the directory listing and return a success code of 0.
  *namesp = directory_list(dnodep);
  return 0;
}
