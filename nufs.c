#include <assert.h>
#include <bsd/string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "slist.h"
#include "storage.h"

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mode)
{
  printf("access(%s, %04o)\n", path, mode);

  // Ensure the path is not null.
  if (!path)
  {
    return -EINVAL;
  }

  // Delegate to storage.
  return storage_access(path, mode);
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *stp)
{
  printf("getattr(%s) {mode: %04o, size: %ld}\n", path, stp->st_mode, stp->st_size);

  // Ensure the path and stat pointer are not null.
  if (!path || !stp)
  {
    return -EINVAL;
  }

  // Delegate to storage.
  return storage_stat(path, stp);
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
  printf("mknod(%s, %04o)\n", path, mode);

  // Ensure the path is not null.
  if (!path)
  {
    return -EINVAL;
  }

  // Delegate to storage.
  return storage_mknod(path, mode);
}

int nufs_link(const char *from, const char *to)
{
  printf("link(%s => %s)\n", from, to);

  // Ensure the from and to paths are not null.
  if (!from || !to)
  {
    return -EINVAL;
  }
  
  // Delegate to storage.
  return storage_link(from, to);
}

int nufs_unlink(const char *path)
{
  printf("unlink(%s)\n", path);

  // Ensure the path is not null.
  if (!path)
  {
    return -EINVAL;
  }

  // Delegate to storage.
  return storage_unlink(path);
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to)
{
  printf("rename(%s => %s)\n", from, to);

  // Ensure the from and to paths are not null.
  if (!from || !to)
  {
    return -EINVAL;
  }
  
  // Delegate to storage.
  return storage_rename(from, to);
}

int nufs_truncate(const char *path, off_t size)
{
  printf("truncate(%s, %ld bytes)\n", path, size);

  // Ensure the path is not null.
  if (!path)
  {
    return -EINVAL;
  }

  // Delegate to storage.
  return storage_truncate(path, size);
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi)
{
  printf("read(%s, %ld bytes, @+%ld)\n", path, size, offset);

  // Ensure the path and buffer are not null.
  if (!path || !buf)
  {
    return -EINVAL;
  }

  // Delegate to storage.
  return storage_read(path, buf, size, offset);
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi)
{
  printf("write(%s, %ld bytes, @+%ld)\n", path, size, offset);

  // Ensure the path and buffer are not null.
  if (!path || !buf)
  {
    return -EINVAL;
  }

  // Delegate to storage.
  return storage_write(path, buf, size, offset);
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode)
{
  printf("mkdir(%s)\n", path);

  // Ensure the path is not null.
  if (!path)
  {
    return -EINVAL;
  }

  // Delegate to mknod but ensure the mode is a directory no matter what.
  return nufs_mknod(path, mode | STORAGE_DIR, 0);
}

int nufs_rmdir(const char *path)
{
  printf("rmdir(%s)\n", path);

  // Ensure the path is not null.
  if (!path)
  {
    return -EINVAL;
  }
  
  // Delegate to storage.
  return storage_rmdir(path);
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi)
{
  printf("readdir(%s)\n", path);
  
  // Ensure the path and filler function pointer are not null.
  if (!path || !filler)
  {
    return -EINVAL;
  }

  slist_t *names = NULL;
  struct stat st;
  int rv;

  rv = storage_list(path, &names);

  // If an issue occured, return this error code.
  if (rv < 0)
  {
    return rv;
  }

  // If the directory is empty, return immediately.
  if (!names)
  {
    slist_free(names);
    return 0;
  }

  slist_t *name_item = names;
  
  while (name_item)
  {
    rv = storage_stat(path, &st);

    if (rv < 0)
    {
      slist_free(names);
      return rv;
    }

    filler(buf, name_item->data, &st, 0);

    name_item = name_item->next;
  }

  slist_free(names);
  return 0;
}

int nufs_chmod(const char *path, mode_t mode)
{
  printf("chmod(%s, %04o)\n", path, mode);
  
  // We are not implementing this, so imply return 0.
  return 0;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi)
{
  printf("open(%s)\n", path);

  int rv = nufs_access(path, F_OK | R_OK | W_OK);

  if (rv < 0)
  {
    return rv;
  }
  
  // We are not implementing this, so imply return 0.
  return 0;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2])
{
  printf("utimens(%s, [%ld, %ld; %ld %ld])\n", path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec,
         ts[1].tv_nsec);

  // We are not implementing this, so imply return 0.
  return 0;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data)
{
  printf("ioctl(%s, %d, ...)\n", path, cmd);
  
  // We are not implementing this, so imply return 0.
  return 0;
}

void nufs_init_ops(struct fuse_operations *ops)
{
  // Zero-out the operation function pointer buffer.
  memset(ops, 0, sizeof(struct fuse_operations));

  // Implemented working versions.
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->mknod = nufs_mknod;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rename = nufs_rename;
  ops->truncate = nufs_truncate;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->mkdir = nufs_mkdir;
  ops->rmdir = nufs_rmdir;
  ops->readdir = nufs_readdir;

  // ops->create   = nufs_create; // alternative to mknod

  // Implemented dummy versions.
  ops->chmod = nufs_chmod;
  ops->open = nufs_open;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[])
{
  assert(argc > 2 && argc < 6);
  
  // Initialize the storage putting the disk image file at the given path.
  storage_init(argv[--argc]);

  // Initialize the fuse operation function pointer buffer and begin fuse.
  nufs_init_ops(&nufs_ops);
  int fuse_exit_code = fuse_main(argc, argv, &nufs_ops, NULL);

  // Deinitialize the storage.
  storage_deinit();

  // Return the exit code fuse produced.
  return fuse_exit_code;
}
