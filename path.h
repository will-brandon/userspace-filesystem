///
/// Utilities for managing paths
///

#ifndef _PATH_H
#define _PATH_H

#include "slist.h"

#define PATH_DELIM '/'

slist_t *path_explode(const char *path);
int inum_for_path_comps_in(int search_root_inum, slist_t *comps);
int inum_for_path_in(int search_root_inum, const char *path);

// Pops off the last non-blank component of the path. Returns the index of that last component
// within the list. Note that the string pointer will be set to a portion of the string allocated in
// the list. When the list data is freed, so is this string.
int path_comps_pop(slist_t *comps, const char **last_comp_name);

// Returns the inum of the parent and the name string of the child (the last non-blank item in the
// path). Note that the child name is on the heap and must later be freed.
int path_parent_child_in(int search_root_inum, const char *path, const char **child_namep);

#endif
