// Inode manipulation routines.

#ifndef _INODE_H
#define _INODE_H

#include "util.h"

#define INODE_FILE 0100000
#define INODE_DIR  0040000

// inode should be total of 32 bytes
#define INODE_LOCAL_BLOCK_CAP 4
#define INODE_MAX_LOCAL_SIZE BLOCK_SIZE * INODE_LOCAL_BLOCK_CAP

// Note that inodes form a tree. If an inode is the child of another inode, its size will likely be
// innacurate since the size of a subnode is not useful information to the algorithms that manage
// these nodes.
typedef struct inode
{
  int refs;                          // reference count (we will leave this at 1)
  int mode;                          // permission & type
  int size;                          // bytes
  int blocks[INODE_LOCAL_BLOCK_CAP]; // <0 if unused, otherwise points to a block
  int next;                          // -1 if no other inode, otherwise inum of next inode
} inode_t;

void inode_print(inode_t *nodep);
void inode_print_tree(inode_t *nodep);
bool_t inode_exists(int inum);
inode_t *inode_get(int inum);
void inode_reset(inode_t *nodep);
int inode_total_size(inode_t *nodep);
inode_t *inode_last_child(inode_t *nodep);
inode_t *inode_second_to_last_child(inode_t *nodep);
int inode_alloc(void);
int inode_free(int inum);
int inode_grow(inode_t *nodep, int size);
int inode_shrink(inode_t *nodep, int size);
int inode_get_bnum(inode_t *nodep, int file_bnum);

#endif
