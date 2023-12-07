#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "specs.h"
#include "bitmap.h"
#include "block.h"
#include "inode.h"
#include "util.h"

bool_t inode_exists(int inum)
{
  assert(inum >= 0);

  return bitmap_get(block_inode_bitmap_start(), inum);
}

inode_t *inode_get(int inum)
{
  assert(inum < MAX_INODE_COUNT);
  assert(inode_exists(inum));

  // Return the proper inode at the given offset.
  return block_inode_start() + (sizeof(inode_t) * inum);
}

void inode_clear(inode_t *nodep)
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
    size += inode_total_size(inode_get(nodep->next));
  }

  return size;
}

inode_t *inode_last_child(inode_t *nodep)
{
  assert(nodep);

  // Recursively follow the chain of inodes until the last one is found.
  return nodep->next < 0 ? nodep : inode_last_child(inode_get(nodep->next));
}

inode_t *inode_second_to_last_child(inode_t *nodep)
{
  assert(nodep);
  assert(nodep->next >= 0);

  inode_t *nextp = inode_get(nodep->next);

  if (nextp->next < 0)
  {
    return nodep;
  }

  return inode_second_to_last_child(nextp);
}

int inode_alloc(void)
{
  void *inode_bitmap = block_inode_bitmap_start();
  inode_t *nodep;

  // Search for the first available inode in the bitmap. If a free inode is found then reset its
  // values.
  for (int inum = 0; inum < MAX_INODE_COUNT; inum++)
  {
    if (!bitmap_get(inode_bitmap, inum))
    {
      nodep = block_inode_start() + (sizeof(inode_t) * inum);
      inode_clear(nodep);
      bitmap_put(inode_bitmap, inum, 1);

      return inum;
    }
  }

  // Return -ENOSPC indicating that there is no space to store more inodes.
  return -ENOSPC;
}

int inode_free(int inum)
{
  assert(inum < MAX_INODE_COUNT);
  assert(inode_exists(inum));

  // Set the inode to unused.
  bitmap_put(block_inode_bitmap_start(), inum, 0);
}

int inode_grow(inode_t *nodep, int size, bool_t zero_out)
{
  assert(nodep);

  // For a growth size of less than 1, return 0 and do nothing. This is a recursive base case.
  if (size < 1)
  {
    return 0;
  }

  // Get the last child inode and see how many blocks it uses.
  inode_t *last_childp = inode_last_child(nodep);
  int last_child_used_blocks = bytes_to_blocks(last_childp->size);

  printf("LAST CHILD CURRENT SIZE: %d\n", last_childp->size);

  printf("NEW BLOCKS NEEDED: %d\n", bytes_to_blocks(last_childp->size + size));

  // If no more blocks are needed simply increase the size.
  if (bytes_to_blocks(last_childp->size + size) == last_child_used_blocks)
  {
    printf("JUST ADDING IN\n");
    last_childp->size += size;
    return size;
  }

  printf("GROWING FROM: %d\n", last_child_used_blocks);

  // If a local slot exists put the new block into it.
  if (last_child_used_blocks < INODE_LOCAL_BLOCK_CAP)
  {
    printf("USING SLOT\n");

    // Allocate the new block and ensure there is enough storage.
    if ((last_childp->blocks[last_child_used_blocks] = block_alloc()) < 0)
    {
      return -ENOSPC;
    }

    // Grow the last child's size by whatever this increment was.
    last_childp->size += MIN(BLOCK_SIZE, size);

    printf("MADE NEW BLOCK (index = %d): %d\n", last_child_used_blocks, last_childp->blocks[last_child_used_blocks]);

    // Recursively grow the inode starting at the last child as a performance shortcut. If the
    // size is not greater than the size of a block nothing will happen.
    if (inode_grow(last_childp, size - BLOCK_SIZE, zero_out) < 0)
    {
      return -ENOSPC;
    }

    printf("GOT HERE, GREW INODE\n");

    // Return the size we successfully incremented by.
    return size;
  }

  // At this point, we fill what we can into this last child and update the remaining size we must
  // find a spot for.
  int remaining_size_change = size - (INODE_MAX_LOCAL_SIZE - last_childp->size);
  last_childp->size = INODE_MAX_LOCAL_SIZE;

  // Since we know we are operating in the last child, we must create a new child.
  if ((last_childp->next = inode_alloc()) < 0)
  {
    return -ENOSPC;
  }

  // Recursively perform the growth in this child inode with the remaining size.
  if (inode_grow(inode_get(last_childp->next), remaining_size_change, zero_out) < 0)
  {
    return -ENOSPC;
  }

  // Return the size.
  return size;
}

int inode_shrink(inode_t *nodep, int size)
{
  assert(nodep);

  // For a shrink size of less than 1 or a node size of 0, return 0 and do nothing. This is a
  // recursive base case. Note that a node size of 0 can only occur at the very top level. All
  // other nodes will be freed if their size ever reaches 0.
  if (size < 1 || nodep->size == 0)
  {
    return 0;
  }

  // Get the last child inode and see how many blocks it uses.
  inode_t *last_childp = inode_last_child(nodep);
  int last_child_used_blocks = bytes_to_blocks(last_childp->size);

  // If no fewer blocks are needed simply decrease the size.
  if (bytes_to_blocks(last_childp->size - size) == last_child_used_blocks)
  {
    last_childp->size -= size;
    return size;
  }

  // Since we know the number of blocks will decrease we can free up the last block and mark that
  // slot as unused.
  block_free(last_childp->blocks[last_child_used_blocks - 1]);
  last_childp->blocks[last_child_used_blocks - 1] = -1;

  // We must calculate carefully the decreased size and use it to further compute the remaining
  // shrinkage that must occur as well as the new last child size.
  int decrease_size = last_childp->size - (BLOCK_SIZE * (last_child_used_blocks - 1));
  int remaining_size_change = size - decrease_size;
  last_childp->size -= decrease_size;

  // If the last child was shrunk all the way down to 0 (i.e. we took everything from the block in
  // the first slot) we can deallocate it.
  if (nodep->next >= 0 && last_childp->size == 0)
  {
    inode_t *second_to_last_childp = inode_second_to_last_child(nodep);
    inode_free(second_to_last_childp->next);
    second_to_last_childp->next = -1;
  }

  // Shrink any remaining size change that is needed.
  inode_shrink(nodep, remaining_size_change);

  return size;
}

int inode_get_bnum(inode_t *nodep, int file_bnum)
{
  assert(nodep);
  assert(file_bnum >= 0);

  // If the file block number is less than the block cap just grab it from the local block list.
  if (file_bnum < INODE_LOCAL_BLOCK_CAP)
  {
    // If the block is not present, -1 will be the value and therefore will get returned which
    // takes care of itself nicely.
    return nodep->blocks[file_bnum];
  }

  // Since we didn't find the block in the local list, return -1 if the next child doesn't exist.
  if (nodep->next < 0)
  {
    return -1;
  }

  // Recursively look into the child.
  return inode_get_bnum(inode_get(nodep->next), file_bnum - INODE_LOCAL_BLOCK_CAP);
}

void inode_print(inode_t *nodep)
{
  assert(nodep);

  printf(
      "INODE(r=%d, m=%o, s=%d, b={%d, %d, %d, %d}, n=%d)\n",
      nodep->refs, nodep->mode, nodep->size,
      nodep->blocks[0], nodep->blocks[1], nodep->blocks[2], nodep->blocks[3],
      nodep->next);
}

void inode_print_tree(inode_t *nodep)
{
  assert(nodep);

  int i, j;

  for (i = 0;; i++)
  {
    if (i > 0)
    {
      repeat_print(" ", 3 * (i - 1));
      printf(" \u2514\u2500");
    }

    inode_print(nodep);

    if (nodep->next < 0)
    {
      return;
    }

    nodep = inode_get(nodep->next);
  }
}

void inode_print_blocks_label_offset(inode_t *nodep, int label_offset)
{
  int block_num;

  for (int i = 0; i < INODE_LOCAL_BLOCK_CAP; i++)
  {
    block_num = nodep->blocks[i];

    if (block_num >= 0)
    {
      printf("\033[0;1;92mBLOCK %d:\033[0m\n", i + label_offset);
      block_print(block_num);
    }
  }

  if (nodep->next >= 0)
  {
    inode_t *nextp = inode_get(nodep->next);
    inode_print_blocks_label_offset(nextp, label_offset + INODE_LOCAL_BLOCK_CAP);
  }
}

void inode_print_blocks(inode_t *nodep)
{
  inode_print_blocks_label_offset(nodep, 0);
}
