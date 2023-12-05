#include "specs.h"
#include "storage.h"
#include "blocks.h"
#include "inode.h"
#include "slist.h"
#include "bitmap.h"

int inode_for_path(const char *path)
{
}

void storage_init(const char *path)
{
  blocks_init(path);

  blocks_clear();

  int inum1 = alloc_inode();
  int inum2 = alloc_inode();
  int inum3 = alloc_inode();
  inode_t *node1p = get_inode(inum1);
  inode_t *node2p = get_inode(inum2);
  inode_t *node3p = get_inode(inum3);

  for (int i = 0; i < 40000; i++)
  {
    grow_inode(node1p, 1);
  }

  for (int i = 0; i < 40000; i++)
  {
    grow_inode(node2p, 1);
  }

  for (int i = 0; i < 40000; i++)
  {
    grow_inode(node1p, 1);
  }

  print_inode_chain(node1p);
  print_inode_chain(node2p);

  for (int i = 0; i < bytes_to_blocks(inode_total_size(node1p)); i++)
  {
    printf("BLOCK %d: %d\n", i, inode_get_bnum(node1p, i));
  }
}

void storage_deinit(void)
{
  blocks_free();
}

int storage_stat(const char *path, struct stat *st)
{
}

int storage_read(const char *path, char *buf, size_t size, off_t offset)
{
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset)
{
}

int storage_truncate(const char *path, off_t size)
{
}

int storage_mknod(const char *path, int mode)
{
}

int storage_unlink(const char *path)
{
}

int storage_link(const char *from, const char *to)
{
}

int storage_rename(const char *from, const char *to)
{
}

int storage_set_time(const char *path, const struct timespec ts[2])
{
}

slist_t *storage_list(const char *path)
{
}
