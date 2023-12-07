///
/// Utilities for managing paths
///

#include <errno.h>
#include <string.h>
#include <assert.h>

#include "path.h"
#include "inode.h"
#include "directory.h"

slist_t *path_explode(const char *path)
{
  assert(path);

  // Simply splot the path by slashes.
  return slist_explode(path, PATH_DELIM);
}

int inum_for_path_comps_in(int search_root_inum, slist_t *comps)
{
  assert(search_root_inum >= 0);
  assert(inode_exists(search_root_inum));

  // Get the node.
  inode_t *dnodep = inode_get(search_root_inum);

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
      return inum_for_path_comps_in(search_root_inum, comps->next);
    }

    return search_root_inum;
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

int inum_for_path_in(int search_root_inum, const char *path)
{
  assert(search_root_inum >= 0);
  assert(inode_exists(search_root_inum));
  assert(inode_get(search_root_inum)->mode & INODE_DIR);
  assert(path);

  // Split the path string into components and delegate to the list version of this function.
  slist_t *comps = path_explode(path);
  int inum = inum_for_path_comps_in(search_root_inum, comps);
  slist_free(comps);

  // Return the inum or error code.
  return inum;
}

int path_comps_pop(slist_t *comps, const char **last_comp_namep)
{
  assert(last_comp_namep);
  
  int i = -1;
  int last_comp_i = -1;
  *last_comp_namep = NULL;

  while (comps)
  {
    i++;

    // If this component is not an empty string, set the last component string pointer to this.
    if (strcmp(comps->data, ""))
    {
      *last_comp_namep = comps->data;
      last_comp_i = i;
    }

    comps = comps->next;
  }

  return last_comp_i;
}

int path_parent_child_in(int search_root_inum, const char *path, const char **child_namep)
{
  assert(search_root_inum >= 0);
  assert(inode_exists(search_root_inum));
  assert(path);
  assert(child_namep);

  // Store the name string of the child, but note that it is a pointer to the component data that
  // will later be freed. This must be duplicated.
  const char *child_name_tethered;

  // Split the path into its components, then get the child name and the index of that child name.
  slist_t *path_comps = path_explode(path);
  int name_comp_i = path_comps_pop(path_comps, &child_name_tethered);

  // Copy the path to the parent directory into a new component list and get the inum.
  slist_t *parent_path_comps = slist_copy(path_comps, name_comp_i);
  int parent_inum = inum_for_path_comps_in(search_root_inum, parent_path_comps);

  // Make a copy of the tethered name.
  *child_namep = strdup(child_name_tethered);

  // Free the path components.
  slist_free(path_comps);
  slist_free(parent_path_comps);

  // Return the parent directory's inum.
  return parent_inum;
}
