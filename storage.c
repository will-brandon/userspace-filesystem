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
#include "path.h"

#define ROOT_INUM 0

static inode_t *root_nodep;

void test(void)
{
  memset(block_get(0) + RESERVED_SIZE, 0xCC, NUFS_SIZE - RESERVED_SIZE);

  int inum1 = inode_alloc();
  int inum2 = inode_alloc();

  inode_t *node1p = inode_get(inum1);
  inode_t *node2p = inode_get(inum2);

  inode_grow(node1p, BLOCK_SIZE / 2);
  inode_grow(node2p, BLOCK_SIZE / 2);
  inode_grow(node1p, BLOCK_SIZE);

  inode_fill(node1p, BLOCK_SIZE - 1, 0x01, 2);

  inode_print_blocks(node1p);
  inode_print_blocks(node2p);

  //inode_print_tree(node1p);
  //inode_print_tree(node2p);

  inode_free(inum1);
  inode_free(inum2);

  /*
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
  directory_add_entry(ROOT_INUM, "school-stuff", inums[2], TRUE);
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

void storage_init(const char *host_path)
{
  assert(host_path);

  // Create a memory map and initialize the disk blocks and.
  block_init(host_path);

  // Allocate the root inode and make it a directory if it doesn't already exist. Note that this
  // relies on ROOT_INUM being 0. Otherwise, there is no guarantee ROOT_INUM will be allocated.
  if (!inode_exists(ROOT_INUM))
  {
    assert(inode_alloc() == ROOT_INUM);
    directory_init(ROOT_INUM);
  }

  // Initialize a pointer to the root node structure and ensure it's .. points to itself.
  root_nodep = inode_get(ROOT_INUM);
  directory_add_entry(ROOT_INUM, "..", ROOT_INUM, FALSE);

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

int storage_inum_for_path(const char *path)
{
  assert(path);

  // Start from the root.
  return inum_for_path_in(ROOT_INUM, path);
}

int storage_path_parent_child(const char *path, const char **child_namep)
{
  assert(path);
  assert(child_namep);
  
  // Start from the root.
  return path_parent_child_in(ROOT_INUM, path, child_namep);
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
  int inum = storage_inum_for_path(path);

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
  int inum = storage_inum_for_path(path);

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

  // Check if an inode already exists at the path.
  int inum = storage_inum_for_path(path);

  // If the path already exists, return an error code.
  if (inum >= 0)
  {
    return EEXIST;
  }

  // If a non-directory node was traversed in the path return this error code.
  if (inum == -ENOTDIR)
  {
    return -ENOTDIR;
  }

  // At this point we better be sure that the result was that it could not be found, otherwise a
  // logical error has occured.
  assert(inum == -ENOENT);

  // IMPORTANT NOTE: At this point, we know that the file this path points to cannot be .. or . or
  // any other type of graph cycle, since it does not exist. Therefore we can assume the very last
  // non-whitespace component in the pathstring is the name, and every single component before it is
  // the directory, whether or not it is canonical does not matter. We will just need to access its
  // inum so we can add an entry into the directory table.

  // Allocate a new node.
  inum = inode_alloc();

  // If an error occured allocating the node, return the error.
  if (inum < 0)
  {
    return inum;
  }

  // Get the inode and update its mode.
  inode_t *nodep = inode_get(inum);
  nodep->mode = mode;

  // If the node is a directory, initialize the directory in the node data.
  if (mode & INODE_DIR)
  {
    directory_init(inum);
  }

  // Get the name of the child and the inum of the parent directory. We know the parent must exist
  // since the inode at the whole path exists.
  const char *name;
  int parent_inum = storage_path_parent_child(path, &name);
  
  // It should be impossile at this point for the parent to not be a directory.
  assert(inode_get(parent_inum)->mode & INODE_DIR);

  // Add the directory entry and free the name buffer now that it has been copied.
  int rv = directory_add_entry(parent_inum, name, inum, TRUE);
  free((void *) name);

  // Return any errors that may have occured attempting to add the directory entry. Ensure we free
  // the inode we created.
  if (rv < 0)
  {
    inode_free(inum);
    return rv;
  }

  // Increase the ref counter and return success code 0.
  nodep->refs++;
  return 0;
}

int storage_link(const char *from, const char *to)
{
  assert(from);
  assert(to);

  // Get the inum of the inode at the "from" path.
  int inum = storage_inum_for_path(from);
  
  // Ensure the inode at the "from" path exists.
  if (inum < 0)
  {
    return -ENOENT;
  }

  // Ensure the inode at the "to" path does not already exist.
  if (storage_inum_for_path(to) >= 0)
  {
    return -EEXIST;
  }

  // Get the name of the child and the inum of the parent directories. We already ensured the "from"
  // parent exists when we found the inode. The "to" parent may not, we must check.
  const char *from_name, *to_name;
  int from_parent_inum = storage_path_parent_child(from, &from_name);
  int to_parent_inum = storage_path_parent_child(to, &to_name);

  // Ensure the "to" parent was properly retrieved. If not, return the error.
  if (to_parent_inum < 0)
  {
    // Be sure we still free the name strings.
    free((void *) from_name);
    free((void *) to_name);

    return to_parent_inum;
  }

  // It should be impossile at this point for either of the parents to not be a directory.
  assert(inode_get(from_parent_inum)->mode & INODE_DIR);
  assert(inode_get(to_parent_inum)->mode & INODE_DIR);

  // Add the directory entry. Hard links arent allowed for directories so we don't have to worry
  // about adding the .. (the last FALSE argument doesn't matter).
  int rv = directory_add_entry(to_parent_inum, to_name, inum, TRUE);

  // Free the name strings.
  free((void *) from_name);
  free((void *) to_name);

  // If the entry failed to be added return its error code.
  if (rv < 0)
  {
    return rv;
  }

  // Increase the ref counter and successfully return 0.
  inode_get(inum)->refs++;
  return 0;
}

int storage_unlink(const char *path)
{
  assert(path);

  // Get the inum of the inode at the path.
  int inum = storage_inum_for_path(path);
  
  // Ensure the inode exists.
  if (inum < 0)
  {
    return -ENOENT;
  }

  // Get the name of the child and the inum of the parent directory. We know the parent must exist
  // since the inode at the whole path exists.
  const char *name;
  int parent_inum = storage_path_parent_child(path, &name);

  inode_t *parent_node = inode_get(parent_inum);
  
  // It should be impossile at this point for the parent to not be a directory.
  assert(parent_node->mode & INODE_DIR);

  // Remove the directory entry. Hard links arent allowed for directories so we don't have to worry
  // about removing the .. (the last FALSE argument doesn't matter).
  int rv = directory_delete(parent_node, name, TRUE);
  free((void *) name);

  // Return any errors that may have occured attempting to remove the directory entry.
  if (rv < 0)
  {
    return rv;
  }

  // Decrease the ref counter.
  int refs = --(inode_get(inum)->refs);

  // If the ref counter is at least 1, return successfully.
  if (refs > 0)
  {
    return 0;
  }
  
  // Free the inode since the ref count dropped below 1.
  rv = inode_free(inum);

  // If the inode could't be freed, return the error code.
  return rv < 0 ? rv : 0;
}

int storage_rename(const char *from, const char *to)
{
  assert(from);
  assert(to);

  int rv = storage_link(from, to);

  if (rv < 0)
  {
    return rv;
  }

  rv = storage_unlink(from);

  if (rv < 0)
  {
    // If we couldn't form the link we need to unlink the new one we already made.
    storage_unlink(to);
    return rv;
  }

  return 0;
}

int storage_rmdir(const char *dpath)
{
  assert(dpath);

  // Lookup the inode at the given path.
  int inum = storage_inum_for_path(dpath);

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

  // Ensure the directory is empty (except . and ..).
  if (!directory_is_empty(dnodep))
  {
    return -ENOTEMPTY;
  }

  // Unlink (and naturally delete) the directory.
  int rv = storage_unlink(dpath);
  return rv < 0 ? rv : 0;
}


int storage_truncate(const char *path, off_t size)
{
  assert(path);
  assert(size >= 0);

  // Lookup the inode inum at the given path.
  int inum = storage_inum_for_path(path);

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
    rv = inode_grow_zero(nodep, size_delta);
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
  buf[0] = 'h';
  buf[1] = 'i';
  buf[2] = '\0';
  buf[3] = EOF;
  printf("BUF: %s\n", buf);
  return 2;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset)
{
  return -1;
}

int storage_list(const char *dpath, slist_t **namesp)
{
  assert(dpath);
  assert(namesp);

  // Lookup the inode at the given path.
  int inum = storage_inum_for_path(dpath);

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
