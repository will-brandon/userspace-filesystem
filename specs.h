///
/// Holds specifications for the filesystem.
///

#ifndef _H_SPECS
#define _H_SPECS

#define BLOCK_SIZE      4096 // 4KB block size
#define BLOCK_COUNT     256  // Split the "disk" into 256 blocks
#define RESERVED_BLOCKS 2    // 2 blocks of size 4KB are reserved for metadata (bitmaps and inodes)
#define MAX_INODE_COUNT 254  // Maximum of 254 inodes allowed because this is all that will fit

#define NUFS_SIZE         (BLOCK_SIZE * BLOCK_COUNT)     // 1MB of total pseudo-disk size
#define RESERVED_SIZE     (BLOCK_SIZE * RESERVED_BLOCKS) // 8192B are reserved (2 blocks)
#define BLOCK_BITMAP_SIZE (BLOCK_COUNT / 8)              // 32B are used to hold the block bitmap
#define INODE_BITMAP_SIZE (MAX_INODE_COUNT / 8)          // 32B are used to hold the inode bitmap

#endif
