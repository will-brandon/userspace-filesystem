#include "util.h"
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

  #define INODES 15

  int inums[INODES];
  inode_t *inodes[INODES];

  for (size_t i = 0; i < INODES; i++)
  {
    inums[i] = alloc_inode();
    inodes[i] = get_inode(inums[i]);

    if (i % 2 == 0)
    {
      directory_init(inums[i]);
    }
  }

  directory_put(inums[0], "main.c", inums[1], TRUE);
  directory_put(inums[0], "main.o", inums[3], TRUE);
  directory_put(inums[0], "hello.txt", inums[5], TRUE);
  directory_put(inums[0], "README.md", inums[7], TRUE);
  directory_put(inums[0], "my stuff", inums[2], TRUE);
  directory_put(inums[2], "empty dir", inums[4], TRUE);
  directory_put(inums[2], "resume.pdf", inums[9], TRUE);

  print_directory(inodes[0], TRUE);
  directory_delete(inodes[0], "hello.txt", TRUE);
  directory_put(inums[0], "a.txt", inums[11], TRUE);
  print_directory(inodes[0], TRUE);
  print_directory(inodes[2], TRUE);
  print_directory(inodes[4], TRUE);
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
