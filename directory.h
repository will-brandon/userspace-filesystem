// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#define DIR_NAME_LENGTH 60

#include "util.h"
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
int directory_total_entry_count(inode_t *dnodep);
int directory_populated_entry_count(inode_t *dnodep);
dirent_t *directory_get_entry(inode_t *dnodep, int entry_num);
int directory_lookup_entry_num(inode_t *dnodep, const char *name);
int directory_lookup_inum(inode_t *dnodep, const char *name);
int directory_rename_entry(inode_t *dnodep, int entry_num, const char *name);
int directory_add_entry(int dinum, const char *name, int entry_inum, bool_t update_child);
int directory_delete(inode_t *dnodep, const char *name, bool_t update_child);
slist_t *directory_list(inode_t *dnodep);
void directory_print(inode_t *dnodep, bool_t include_empty_entries);

#endif
