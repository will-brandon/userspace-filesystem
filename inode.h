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
  int refs;                          // reference/link count
  int mode;                          // permission & type
  int size;                          // bytes
  int blocks[INODE_LOCAL_BLOCK_CAP]; // <0 if unused, otherwise points to a block
  int next;                          // -1 if no other inode, otherwise inum of next inode
} inode_t;

// Define a block iterator for reading, writing, filling, etc.
typedef int (* block_iter_t)(void *buf, void *start, int offset, int size);

bool_t inode_exists(int inum);
inode_t *inode_get(int inum);
void inode_reset(inode_t *nodep);
int inode_total_size(inode_t *nodep);
inode_t *inode_last_child(inode_t *nodep);
inode_t *inode_second_to_last_child(inode_t *nodep);
int inode_alloc(void);
int inode_free(int inum);
int inode_clear(inode_t *nodep);
int inode_grow(inode_t *nodep, int size);
int inode_grow_zero(inode_t *nodep, int size);
int inode_shrink(inode_t *nodep, int size);
int inode_get_bnum(inode_t *nodep, int file_bnum);
void *inode_end(inode_t *nodep);
int inode_block_iter(inode_t *nodep, block_iter_t iter, void *buf, int offset, int size);
int inode_fill(inode_t *nodep, int offset, byte_t fill, int size);
void inode_print(inode_t *nodep);
void inode_print_tree(inode_t *nodep);
void inode_print_blocks(inode_t *nodep);
void inode_print_bitmap(void);

#endif
