#include "specs.h"
#include "storage.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"
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

  printf("BEFORE: %o\n", node1p->mode);

  directory_init(inum1);

  printf("BEFORE: %o\n", node1p->mode);

  print_directory(node1p);
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
