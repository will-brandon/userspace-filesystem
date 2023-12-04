// Inode manipulation routines.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code
#ifndef INODE_H
#define INODE_H

#include "blocks.h"

typedef struct inode
{
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int block[4]; // 0 if unused, >0 if points to a block
  int next; // -1 if no other inode, otherwise index of next inode within inode block
} inode_t;

void print_inode(inode_t *node);
inode_t *get_inode(int inum);
int alloc_inode(void);
void free_inode(void);
int grow_inode(inode_t *node, int size);
int shrink_inode(inode_t *node, int size);
int inode_get_bnum(inode_t *node, int file_bnum);

#endif
