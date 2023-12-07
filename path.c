///
/// Utilities for managing paths
///

#include <errno.h>
#include <string.h>
#include <assert.h>

#include "path.h"
#include "inode.h"
#include "directory.h"

int inum_for_path_comps_in(int dinum, slist_t *comps)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));

  // Get the node.
  inode_t *dnodep = inode_get(dinum);

  assert(dnodep->mode & INODE_DIR);

  // If the components are null return no such file error.
  if (!comps)
  {
    return -ENOENT;
  }

  // Ensure the data is not null.
  assert(comps->data);

  // If the string is empty and the next token exists, i.e. double slash, ignore it and move on to
  // the next token. But, if the string is empty and there is no next token, return the parent dir.
  if (!strcmp(comps->data, ""))
  {
    if (comps->next)
    {
      return inum_for_path_comps_in(dinum, comps->next);
    }

    return dinum;
  }

  int entry_inum;

  // Lookup the inode in the directory. If it is an error, we can conveniently return this error
  // code. If the lookup succeeded and this is the end of the path, return the entry.
  if ((entry_inum = directory_lookup_inum(dnodep, comps->data)) < 0 || !comps->next)
  {
    return entry_inum;
  }

  // We ensure the entry node is a directory since there is more path coming.
  if (!(inode_get(entry_inum)->mode & INODE_DIR))
  {
    return -ENOTDIR;
  }

  // Continue on down the path looking in that directory.
  return inum_for_path_comps_in(entry_inum, comps->next);
}

int inum_for_path_in(int dinum, const char *path)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));
  assert(inode_get(dinum)->mode & INODE_DIR);
  assert(path);

  // Split the path string into components and delegate to the list version of this function.
  slist_t *comps = slist_explode(path, '/');
  int inum = inum_for_path_comps_in(dinum, comps);
  slist_free(comps);

  // Return the inum or error code.
  return inum;
}

int parent_inum_for_path_in(int dinum, const char *path)
{
  assert(dinum >= 0);
  assert(inode_exists(dinum));
  assert(inode_get(dinum)->mode & INODE_DIR);
  assert(path);

  // Split the path string into components.
  slist_t *comps = slist_explode(path, '/');

  // If the path is either
  if (!comps || !comps->next)
  {
    return -ENOENT;
  }

  slist_t *comp = comps;

  while (comp)
  {

  }

  int inum = parent_inum_for_path_comps_in(dinum, comps);
  slist_free(comps);

  // Return the inum or error code.
  return inum;
}

void path_parent(char *path)
{
  assert(path);

  int len = strlen(path);
  int i;

  for (i = len - 1; i >= 0 && path[i] == '/'; i--) {}
}
