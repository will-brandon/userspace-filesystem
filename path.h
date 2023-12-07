///
/// Utilities for managing paths
///

#ifndef _PATH_H
#define _PATH_H

#include "slist.h"

int inum_for_path_comps_in(int dinum, slist_t *comps);
int inum_for_path_in(int dinum, const char *path);

// Pops off the last non-whitesapce component of the path. Returns the index of that last component
// within the list. Note that the string pointer will be set to a portion of the string allocated in
// the list. When the list data is freed, so is this string.
int path_comps_pop(slist_t *comps, const char **last_comp_name);

#endif
