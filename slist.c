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
  slist_t *xs = malloc(sizeof(slist_t));
  xs->data = strdup(text);
  xs->next = rest;
  return xs;
}

void slist_free(slist_t *xs)
{
  if (xs == NULL)
  {
    return;
  }

  slist_free(xs->next);
  free(xs->data);
  free(xs);
}

slist_t *slist_explode(const char *text, char delim)
{
  assert(text);

  // If the character is string null terminator return NULL.
  if (*text == '\0')
  {
    return NULL;
  }

  int plen = 0;
  while (text[plen] != '\0' && text[plen] != delim)
  {
    plen += 1;
  }

  int skip = 0;
  if (text[plen] == delim)
  {
    skip = 1;
  }

  slist_t *rest = slist_explode(text + plen + skip, delim);
  char *part = alloca(plen + 2);
  memcpy(part, text, plen);
  part[plen] = 0;

  return slist_cons(part, rest);
}

int slist_size(slist_t *xs)
{
  if (!xs)
  {
    return 0;
  }

  return 1 + slist_size(xs->next);
}

void slist_print(slist_t *xs, const char *delim)
{
  if (xs == NULL)
  {
    return;
  }

  printf("%s", xs->data);

  if (xs->next)
  {
    printf("%s", delim);
    slist_print(xs->next, delim);
  }
}
