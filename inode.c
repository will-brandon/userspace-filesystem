#include <stdio.h>
#include "specs.h"
#include "bitmap.h"
#include "blocks.h"
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
    // Ensure the inum is in range and actually exists.
    if (inum >= MAX_INODE_COUNT || !bitmap_get(get_inode_bitmap(), inum))
    {
        return NULL;
    }

    // Return the proper inode at the given offset.
    return get_inode_start() + (sizeof(inode_t) * inum);
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
