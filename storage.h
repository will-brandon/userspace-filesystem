
#ifndef _STORAGE_H
#define _STORAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "slist.h"

#define STORAGE_FILE 0100000
#define STORAGE_DIR  0040000

void storage_init(const char *host_path);
void storage_deinit(void);
void storage_clear(void);
int storage_inum_for_path(const char *path);
int storage_path_parent_child(const char *path, const char **child_name);
int storage_access(const char *path, int mode);
int storage_stat(const char *path, struct stat *st);
int storage_mknod(const char *path, int mode);
int storage_link(const char *from, const char *to);
int storage_unlink(const char *path);
int storage_rename(const char *from, const char *to);
int storage_rmdir(const char *dpath);
int storage_truncate(const char *path, off_t size);
int storage_read(const char *path, char *buf, size_t size, off_t offset);
int storage_write(const char *path, const char *buf, size_t size, off_t offset);
int storage_list(const char *dpath, slist_t **namesp);

#endif
