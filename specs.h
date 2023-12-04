///
/// Holds specifications for the filesystem.
///

#ifndef _H_SPECS
#define _H_SPECS

#define BLOCK_COUNT     256  // Split the "disk" into 256 blocks
#define BLOCK_SIZE      4096 // 4KB block size
#define MAX_INODE_COUNT 256  // Maximum of 256 inodes allowed

#define NUFS_SIZE         (BLOCK_SIZE * BLOCK_COUNT) // 1MB of total pseudo-disk size
#define BLOCK_BITMAP_SIZE (BLOCK_COUNT / 8)          // 32B will be used to hold the block bitmap
#define INODE_BITMAP_SIZE (MAX_INODE_COUNT / 8)      // 32B will be used to hold the block bitmap

#endif
