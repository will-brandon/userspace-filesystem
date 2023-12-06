#define _GNU_SOURCE

#include <stdlib.h>
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

#define ROOT_INUM 0

static inode_t *root_nodep;

void test(void)
{
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

  printf("%d\n", directory_delete(inodes[6], "hw2.pdf", TRUE));

  directory_print(root_nodep, TRUE);
  directory_print(inodes[0], TRUE);
  directory_print(inodes[2], TRUE);
  directory_print(inodes[4], TRUE);
  directory_print(inodes[6], TRUE);
  directory_print(inodes[8], TRUE);
  directory_print(inodes[10], TRUE);
  directory_print(inodes[12], TRUE);
  directory_print(inodes[14], TRUE);
  
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
  if (!(get_inode(entry_inum)->mode & INODE_DIR))
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
  assert(get_inode(dinum)->mode & INODE_DIR);
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
  blocks_init(path);

  printf("\033[0;1;31mREMEMBER TO REMOVE THIS CLEAR FUNCTION DUMBASS!\033[0m\n");
  blocks_clear();

  // Allocate the root inode and make it a directory if it doesn't already exist. Note that this
  // relies on ROOT_INUM being 0. Otherwise, there is no guarantee ROOT_INUM will be allocated.
  if (!inode_exists(ROOT_INUM))
  {
    assert(alloc_inode() == ROOT_INUM);
    directory_init(ROOT_INUM);
  }

  // Initialize a pointer to the root node structure.
  root_nodep = get_inode(ROOT_INUM);

  test();
}

void storage_deinit(void)
{
  // Persist the blocks in the memory map to disk .
  blocks_free();
}

int storage_access(const char *path, int mode)
{
  assert(path);

  // If none of the modes were requested, return a success code of 0.
  if (!(mode & F_OK & R_OK & W_OK & X_OK))
  {
    return 0;
  }

  // Lookup the inode at the given path.
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

  // Lookup the inode at the given path.
  int inum = inum_for_path(path);

  // If a lookup error occured return the error code.
  if (inum < 0)
  {
    return inum;
  }

  // Get a pointer to the inode.
  inode_t *nodep = get_inode(inum);

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

int storage_list(const char *path, slist_t **namesp)
{
  assert(path);

  // Lookup the inode at the given path.
  int inum = inum_for_path(path);

  // If a lookup error occured return the error code.
  if (inum < 0)
  {
    return inum;
  }

  // Get a pointer to the inode.
  inode_t *dnodep = get_inode(inum);

  // If the node is not a directory return an error code.
  if (!(dnodep->mode & INODE_DIR))
  {
    return -ENOTDIR;
  }

  // Get the directory listing and return a success code of 0.
  *namesp = directory_list(dnodep);
  return 0;
}
