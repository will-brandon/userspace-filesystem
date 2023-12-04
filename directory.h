// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

// MAKE SURE THAT . AND MAYBE EVEN .. ARE THE FIRST TWO ENTRIES
typedef struct dirent {
  char name[DIR_NAME_LENGTH];
  int inum; // Make -1 if unused
  char _reserved[12];
} dirent_t;

void directory_init(void);
int directory_lookup(inode_t *di, const char *name);
int directory_put(inode_t *di, const char *name, int inum);
int directory_delete(inode_t *di, const char *name);
slist_t *directory_list(const char *path);
void print_directory(inode_t *dd);

#endif
