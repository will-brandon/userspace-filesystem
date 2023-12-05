// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#define DIR_NAME_LENGTH 60

#include "blocks.h"
#include "inode.h"
#include "slist.h"

// Size of exactly 64
typedef struct dirent {
  char name[DIR_NAME_LENGTH]; // Note that this string is guarunteed to have a null terminator for
                              // safety. This means its content length is DIR_NAME_LENGTH - 1.
  int inum;                   // -1 if unused
} dirent_t;

void directory_init(int inum);
int directory_entry_count(inode_t *nodep);
dirent_t *directory_get(inode_t *nodep, int i);
int directory_rename(dirent_t *entryp, const char *name);
int directory_lookup(inode_t *nodep, const char *name);
int directory_lookup_path(inode_t *nodep, const char *path);
int directory_put(inode_t *nodep, const char *name, int inum);
int directory_delete(inode_t *nodep, const char *name);
slist_t *directory_list(const char *path);
void print_directory(inode_t *nodep);

#endif
