// Inode manipulation routines.

#ifndef _INODE_H
#define _INODE_H

#define INODE_FILE 0100000
#define INODE_DIR  0040000

// inode should be total of 32 bytes
#define INODE_LOCAL_BLOCK_CAP 4

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

void print_inode(inode_t *nodep);
inode_t *get_inode(int inum);
void clear_inode(inode_t *nodep);
int inode_local_available_block_slot(inode_t *nodep);
int alloc_inode(void);
int free_inode(int inum);
int grow_inode(inode_t *nodep, int size);
int shrink_inode(inode_t *nodep, int size);
int inode_get_bnum(inode_t *nodep, int file_bnum);

#endif
