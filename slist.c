/**
 * @file slist.c
 * @author CS3650 staff
 *
 * A simple linked list of strings.
 *
 * This might be useful for directory listings and for manipulating paths.
 */

#include <assert.h>
#include <alloca.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "slist.h"

slist_t *slist_cons(const char *text, slist_t *rest)
{
  slist_t *sl = malloc(sizeof(slist_t));
  sl->data = strdup(text);
  sl->next = rest;
  return sl;
}

void slist_free(slist_t *sl)
{
  if (!sl)
  {
    return;
  }

  slist_free(sl->next);
  free(sl->data);
  free(sl);
}

int slist_size(slist_t *sl)
{
  if (!sl)
  {
    return 0;
  }

  return 1 + slist_size(sl->next);
}

slist_t *slist_copy(slist_t *sl, int end)
{
  if (!sl || end <= 0)
  {
    return NULL;
  }

  slist_t *subset = slist_cons(sl->data, NULL);
  slist_t *head = subset;

  for (int i = 1; i < end && sl->next; i++)
  {
    sl = sl->next;
    head->next = slist_cons(sl->data, NULL);
    head = head->next;
  }

  return subset;
}

void slist_print(slist_t *sl, const char *delim)
{
  if (sl == NULL)
  {
    return;
  }

  printf("%s", sl->data);

  if (sl->next)
  {
    printf("%s", delim);
    slist_print(sl->next, delim);
  }
}

slist_t *slist_explode(const char *text, char delim)
{
  assert(text);

  // If the character is string null terminator return NULL.
  if (*text == '\0')
  {
    return NULL;
  }

  int next = 0;
  while (text[next] != '\0' && text[next] != delim)
  {
    next += 1;
  }

  int skip = 0;
  if (text[next] == delim)
  {
    skip = 1;
  }

  slist_t *rest = slist_explode(text + next + skip, delim);
  char *part = alloca(next + 2);
  memcpy(part, text, next);
  part[next] = 0;

  return slist_cons(part, rest);
}
