// Inode manipulation routines.

#ifndef _INODE_H
#define _INODE_H

#define INODE_FILE 0100000
#define INODE_DIR  0040000

// inode should be total of 32 bytes
#define INODE_BLOCK_COUNT 4

typedef struct inode
{
  int refs;                      // reference count (we will leave this at 1)
  int mode;                      // permission & type
  int size;                      // bytes
  int blocks[INODE_BLOCK_COUNT]; // <0 if unused, otherwise points to a block
  int next;                      // -1 if no other inode, otherwise index of next inode within inode block
} inode_t;

void print_inode(inode_t *nodep);
inode_t *get_inode(int inum);
void clear_inode(inode_t *nodep);
int alloc_inode(void);
int free_inode(int inum);
int grow_inode(inode_t *nodep, int size);
int shrink_inode(inode_t *nodep, int size);
int inode_get_bnum(inode_t *nodep, int file_bnum);

#endif
