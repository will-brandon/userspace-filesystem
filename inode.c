#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "specs.h"
#include "bitmap.h"
#include "blocks.h"
#include "inode.h"

void print_inode(inode_t *nodep)
{
    assert(nodep);
    
    printf(
        "INODE(r=%d, m=%o, s=%d, b={%d, %d, %d, %d}, n=%d)\n",
        nodep->refs, nodep->mode, nodep->size,
        nodep->blocks[0], nodep->blocks[1], nodep->blocks[2], nodep->blocks[3],
        nodep->next);
}

inode_t *get_inode(int inum)
{
    assert(inum < MAX_INODE_COUNT);
    assert(bitmap_get(get_inode_bitmap(), inum));

    // Return the proper inode at the given offset.
    return get_inode_start() + (sizeof(inode_t) * inum);
}

void clear_inode(inode_t *nodep)
{
    assert(nodep);

    nodep->refs = 0;
    nodep->mode = 0100644;
    nodep->size = 0;
    memset(nodep->blocks, -1, sizeof(int) * INODE_LOCAL_BLOCK_CAP);
    nodep->next = -1;
}

int inode_block_count(inode_t *nodep)
{
    assert(nodep);

    // Begin a count.
    int count = 0;

    // Check how many local blocks there are.
    for (int i = 0; i < INODE_LOCAL_BLOCK_CAP; i++)
    {
        if (nodep->blocks[i] >= 0)
        {
            count++;
        }
    }

    // If there is no next block, simply return the local count.
    if (nodep->next < 0)
    {
        return count;
    }

    // Since a next block exists, return the local count plus the count of the next block.
    return count + inode_block_count(get_inode(nodep->next));
}

int inode_available_block(inode_t *nodep)
{
    assert(nodep);

    // Find the index of the first available block in the range [0, INODE_BLOCK_COUNT).
    for (int i = 0; i < INODE_LOCAL_BLOCK_CAP; i++)
    {
        if (nodep->blocks[i] < 0)
        {
            return i;
        }
    }

    // If no blocks are available return -1.
    return -1;
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

    // Return -1 indicating that there is no space to store more inodes.
    return -1;
}

int free_inode(int inum)
{
    assert(inum < MAX_INODE_COUNT);
    assert(bitmap_get(get_inode_bitmap(), inum));

    // Set the inode to unused.
    bitmap_put(get_inode_bitmap(), inum, 0);
}

int grow_inode(inode_t *nodep, int size)
{
    assert(nodep);

    // If a size of less than 1 is given return 0 and do nothing. This is a recursive base case.
    if (size < 1)
    {
        return 0;
    }

    int new_block_count = bytes_to_blocks(nodep->size + size);
    int available_block = inode_available_block(nodep);

    if (available_block >= 0)
    {

    }

    if (available_block < 0)
    {
        if (nodep->next < 0 && (nodep->next = alloc_inode()) < 0)
        {
            return -1;
        }


    }
}

int shrink_inode(inode_t *nodep, int size)
{

}

int inode_get_bnum(inode_t *nodep, int file_bnum)
{

}
