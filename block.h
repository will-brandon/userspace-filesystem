/**
 * @file block.h
 * @author CS3650 staff
 *
 * A block-based abstraction over a disk image file.
 *
 * The disk image is mmapped, so block data is accessed using pointers.
 */
#ifndef _BLOCK_H
#define _BLOCK_H

#include <stdio.h>

/** 
 * Compute the number of blocks needed to store the given number of bytes.
 *
 * @param bytes Size of data to store in bytes.
 *
 * @return Number of blocks needed to store the given number of bytes.
 */
int bytes_to_blocks(int bytes);

/**
 * Load and initialize the given disk image.
 *
 * @param image_path Path to the disk image file.
 */
void block_init(const char *image_path);

/**
 * Close the disk image.
 */
void block_deinit(void);

/**
 * Clears all blocks but still keeps the first RESERVED_BLOCKS blocks reserved.
 */
void block_clear(void);

/**
 * Get the block with the given index, returning a pointer to its start.
 *
 * @param bnum Block number (index).
 *
 * @return Pointer to the beginning of the block in memory.
 */
void *block_get(int bnum);

/**
 * Return a pointer to the beginning of the block bitmap.
 *
 * @return A pointer to the beginning of the free blocks bitmap.
 */
void *block_block_bitmap_start(void);

/**
 * Return a pointer to the beginning of the inode table bitmap.
 *
 * @return A pointer to the beginning of the free inode bitmap.
 */
void *block_inode_bitmap_start(void);

void *block_inode_start(void);

void *block_content_start(void);

/**
 * Allocate a new block and return its number.
 *
 * Grabs the first unused block and marks it as allocated.
 *
 * @return The index of the newly allocated block.
 */
int block_alloc(void);

/**
 * Deallocate the block with the given number.
 *
 * @param bnun The block number to deallocate.
 */
void block_free(int bnum);

void block_print(int bnum);

#endif
