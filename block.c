/**
 * @file block.c
 * @author CS3650 staff
 *
 * Implementatino of a block-based abstraction over a disk image file.
 */

#define _GNU_SOURCE
#include <string.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"
#include "specs.h"
#include "bitmap.h"
#include "block.h"

#define BLOCK_PRINT_COLS 32

static int blocks_fd = -1;
static void *blocks_base = 0;

// Get the number of blocks needed to store the given number of bytes.
int bytes_to_blocks(int bytes)
{
  return (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

// Load and initialize the given disk image.
void block_init(const char *image_path)
{
  blocks_fd = open(image_path, O_CREAT | O_RDWR, 0644);
  assert(blocks_fd != -1);

  // make sure the disk image is exactly 1MB
  int rv = ftruncate(blocks_fd, NUFS_SIZE);
  assert(rv == 0);

  // map the image to memory
  blocks_base =
      mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, blocks_fd, 0);
  assert(blocks_base != MAP_FAILED);

  // block 0 stores the block bitmap and the inode bitmap
  void *bbm = block_block_bitmap_start();

  // Mark both blocks as occupied
  bitmap_put(bbm, 0, 1);
  bitmap_put(bbm, 1, 1);
}

// Close the disk image.
void block_deinit(void)
{
  int rv = munmap(blocks_base, NUFS_SIZE);
  assert(rv == 0);
}

void block_clear(void)
{
  // Memory clear everything.
  memset(blocks_base, 0, NUFS_SIZE);

  // block 0 stores the block bitmap and the inode bitmap
  void *bbm = block_block_bitmap_start();

  // Mark both blocks as occupied
  bitmap_put(bbm, 0, 1);
  bitmap_put(bbm, 1, 1);
}

// Get the given block, returning a pointer to its start.
void *block_get(int bnum)
{
  return blocks_base + (BLOCK_SIZE * bnum);
}

// Return a pointer to the beginning of the block bitmap.
// The size is BLOCK_BITMAP_SIZE bytes.
void *block_block_bitmap_start(void)
{
  return block_get(0);
}

// Return a pointer to the beginning of the inode table bitmap.
void *block_inode_bitmap_start(void)
{
  // The inode bitmap is stored immediately after the block bitmap
  return block_block_bitmap_start() + BLOCK_BITMAP_SIZE;
}

void *block_inode_start(void)
{
  return block_inode_bitmap_start() + INODE_BITMAP_SIZE;
}

void *block_content_start(void)
{
  return block_get(0) + RESERVED_SIZE;
}

// Allocate a new block and return its index.
int block_alloc(void)
{
  void *bbm = block_block_bitmap_start();

  for (int ii = RESERVED_BLOCKS; ii < BLOCK_COUNT; ++ii) {
    if (!bitmap_get(bbm, ii)) {
      bitmap_put(bbm, ii, 1);

      printf("block_alloc() -> %d\n", ii);

      return ii;
    }
  }

  return -ENOSPC;
}

// Deallocate the block with the given index.
void block_free(int bnum)
{
  printf("block_free(%d)\n", bnum);
  void *bbm = block_block_bitmap_start();
  bitmap_put(bbm, bnum, 0);
}

void block_print(int bnum)
{
  assert(bnum >= 0);

  const int rows = BLOCK_SIZE / BLOCK_PRINT_COLS;

  void *bytep = block_get(bnum);
  unsigned int byte_buffer;

  for (int i = 0; i < BLOCK_SIZE; i++)
  {
    byte_buffer = *((byte_t *) bytep);

    // Ensure an extra 0 is added if necessary.
    if (byte_buffer > 0xF)
    {
      printf("%x", byte_buffer);
    }
    else
    {
      printf("0%x", byte_buffer);
    }

    if ((i + 1) % BLOCK_PRINT_COLS == 0)
    {
      printf("\n");
    }
    else
    {
      printf(" ");
    }

    bytep++;
  }
}
