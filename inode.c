#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "specs.h"
#include "bitmap.h"
#include "blocks.h"
#include "inode.h"
#include "util.h"

void print_inode(inode_t *nodep)
{
    assert(nodep);
    
    printf(
        "INODE(r=%d, m=%o, s=%d, b={%d, %d, %d, %d}, n=%d)\n",
        nodep->refs, nodep->mode, nodep->size,
        nodep->blocks[0], nodep->blocks[1], nodep->blocks[2], nodep->blocks[3],
        nodep->next);
}

void print_inode_chain(inode_t *nodep)
{
    assert(nodep);

    int i, j;

    for (i = 0 ;; i++)
    {
        for (j = 0; j < i; j++)
        {
            printf(" ");
        }
        if (j > 0)
        {
            printf("\u2514");
        }

        print_inode(nodep);

        if (nodep->next < 0)
        {
            return;
        }

        nodep = get_inode(nodep->next);
    }
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

int inode_total_size(inode_t *nodep)
{
    assert(nodep);

    int size = nodep->size;

    if (nodep->next >= 0)
    {
        size += inode_total_size(get_inode(nodep->next));
    }

    return size;
}

int inode_last_child_inum(int inum)
{
    assert(inum >= 0);
    assert(bitmap_get(get_inode_bitmap(), inum));

    inode_t *nodep = get_inode(inum);

    return nodep->next < 0 ? inum : inode_last_child_inum(nodep->next);
}

inode_t *inode_last_child(inode_t *nodep)
{
    assert(nodep);

    return nodep->next < 0 ? nodep : inode_last_child(get_inode(nodep->next));
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

    // Get the last child inode and see how many blocks it uses.
    inode_t *last_childp = inode_last_child(nodep);
    int last_child_used_blocks = bytes_to_blocks(last_childp->size);
    
    // If no more blocks are needed simply increase the size.
    if (bytes_to_blocks(last_childp->size + size) == last_child_used_blocks)
    {
        last_childp->size += size;
        return size;
    }

    // If a local slot exists put the new block into it.
    if (last_child_used_blocks < INODE_LOCAL_BLOCK_CAP)
    {
        // Allocate the new block and ensure there is enough storage.
        if ((last_childp->blocks[last_child_used_blocks] = alloc_block()) < 0)
        {
            return -1;
        }

        // Recursively grow the inode starting at the last child as a performance shortcut. If the
        // size is not greater than the size of a block nothing will happen.
        if (grow_inode(last_childp, size - BLOCK_SIZE) < 0)
        {
            return -1;
        }

        // Grow the last child's size by whatever this increment was.
        last_childp->size += MIN(BLOCK_SIZE, size);
        return size;
    }

    // At this point, we fill what we can into this last child and update the remaining size we must
    // find a spot for.
    int remaining_size_change = size - (INODE_MAX_LOCAL_SIZE - last_childp->size);
    last_childp->size = INODE_MAX_LOCAL_SIZE;

    // Since we know we are operating in the last child, we must create a new child.
    if ((last_childp->next = alloc_inode()) < 0)
    {
        return -1;
    }

    // Recursively perform the growth in this child inode with the remaining size.
    if (grow_inode(get_inode(last_childp->next), remaining_size_change) < 0)
    {
        return -1;
    }

    // Return the size.
    return size;
}

int shrink_inode(inode_t *nodep, int size)
{
    assert(nodep);

    // If a size of less than 1 is given return 0 and do nothing. This is a recursive base case.
    if (size < 1)
    {
        return 0;
    }

    // Get the last child inode and see how many blocks it uses.
    int last_child_inum = inode_last_child_inum(nodep);
    inode_t *last_childp = get_inode(last_child_inum);
    int last_child_used_blocks = bytes_to_blocks(last_childp->size);
    
    // If no fewer blocks are needed simply decrease the size.
    if (bytes_to_blocks(last_childp->size - size) == last_child_used_blocks)
    {
        last_childp->size -= size;
        return size;
    }
    /*
    // Since we know the number of blocks will decrease we can free up the last block.
    free_block(last_childp->blocks[last_child_used_blocks - 1]);
    last_childp->blocks[last_child_used_blocks - 1] = -1;
    int decrease_size = last_childp->size - (BLOCK_SIZE * (last_child_used_blocks - 1));
    int remaining_size_change = size - decrease_size;
    last_childp->size -= decrease_size;

    if (last_childp->size == 0)
    {
        free_inode(last_childp);
    }*/

    return size;
}

int inode_get_bnum(inode_t *nodep, int file_bnum)
{

}
