#include <stdio.h>
#include "inode.h"

void print_inode(inode_t *node)
{
    printf(
        "INODE(r=%d, m=%d, s=%d, b={%d, %d, %d, %d}, n=%d)\n",
        node->refs, node->mode, node->size,
        node->block[0], node->block[1], node->block[2], node->block[3],
        node->next);
}

inode_t *get_inode(int inum)
{
    
}

int alloc_inode(void)
{

}

void free_inode(void)
{

}

int grow_inode(inode_t *node, int size)
{

}

int shrink_inode(inode_t *node, int size)
{

}

int inode_get_bnum(inode_t *node, int file_bnum)
{

}
