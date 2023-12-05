#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "specs.h"
#include "bitmap.h"
#include "blocks.h"
#include "inode.h"

void print_inode(inode_t *nodep)
{
    printf(
        "INODE(r=%d, m=%o, s=%d, b={%d, %d, %d, %d}, n=%d)\n",
        nodep->refs, nodep->mode, nodep->size,
        nodep->blocks[0], nodep->blocks[1], nodep->blocks[2], nodep->blocks[3],
        nodep->next);
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

void clear_inode(inode_t *nodep)
{
    nodep->refs = 0;
    nodep->mode = 0100644;
    nodep->size = 0;
    memset(nodep->blocks, -1, sizeof(int) * INODE_BLOCK_COUNT);
    nodep->next = -1;
}

int alloc_inode(void)
{
    void *inode_bitmap = get_inode_bitmap();
    inode_t *nodep;

    // Search for the first available inode in the bitmap. If a free inode is found then reset its
    // values.
    for (int inum = 0; inum < MAX_INODE_COUNT; inum++)
    {
        if (!bitmap_get(inode_bitmap, inum))
        {
            nodep = get_inode_start() + (sizeof(inode_t) * inum);
            clear_inode(nodep);
            bitmap_put(inode_bitmap, inum, 1);

            return inum;
        }
    }

    // Return -1 if no more inodes can be allocated because the maximum amount has been reached.
    return -1;
}

int free_inode(int inum)
{
    // Ensure the inum is in range and was actually in use.
    if (inum >= MAX_INODE_COUNT || !bitmap_get(get_inode_bitmap(), inum))
    {
        return -1;
    }

    // Set the inode to unused.
    bitmap_put(get_inode_bitmap(), inum, 0);
}

int grow_inode(inode_t *nodep, int size)
{

}

int shrink_inode(inode_t *nodep, int size)
{

}

int inode_get_bnum(inode_t *nodep, int file_bnum)
{

}
