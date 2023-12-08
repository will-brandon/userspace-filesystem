// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#define MAX_DIR_ENTRY_NAME_LEN 60

#include "util.h"
#include "block.h"
#include "inode.h"
#include "slist.h"

// Size of exactly 64
typedef struct dirent {
  char name[MAX_DIR_ENTRY_NAME_LEN]; // Note that this string is guarunteed to have a null
                                     // terminator for safety. This means its content length is
                                     // MAX_DIR_ENTRY_NAME_LEN - 1.
  int inum; // -1 if unused
} dirent_t;

void directory_init(int inum);
int directory_total_entry_count(inode_t *dnodep);
int directory_populated_entry_count(inode_t *dnodep);
bool_t directory_is_empty(inode_t *dnodep);
dirent_t *directory_get_entry(inode_t *dnodep, int entry_num);
int directory_lookup_entry_num(inode_t *dnodep, const char *name);
int directory_lookup_inum(inode_t *dnodep, const char *name);
int directory_rename_entry(inode_t *dnodep, int entry_num, const char *name);
int directory_add_entry(int dinum, const char *name, int entry_inum, bool_t back_entry_in_child);
int directory_remove_entry(inode_t *dnodep, const char *name, bool_t back_entry_in_child);
slist_t *directory_list(inode_t *dnodep);
void directory_print_entries(inode_t *dnodep, bool_t include_empty_entries);
void directory_print_tree(inode_t *dnodep);

#endif
