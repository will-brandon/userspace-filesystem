///
/// Utilities for managing paths
///

#ifndef _PATH_H
#define _PATH_H

#include "slist.h"

int inum_for_path_comps_in(int dinum, slist_t *comps);
int inum_for_path_in(int dinum, const char *path);
int parent_inum_for_path_comps_in(int dinum, slist_t *comps);
int parent_inum_for_path_in(int dinum, const char *path);

#endif