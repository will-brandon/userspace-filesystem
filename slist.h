/**
 * @file slist.h
 * @author CS3650 staff
 *
 * A simple linked list of strings.
 *
 * This might be useful for directory listings and for manipulating paths.
 */
#ifndef _SLIST_H
#define _SLIST_H

typedef struct slist
{
  char *data;
  struct slist *next;
} slist_t;

/**
 * Cons a string to a string list.
 *
 * @param text String to cons on to a list
 * @param rest List of strings to cons onto.
 *
 * @return List starting with the given string in front of the original list.
 */
slist_t *slist_cons(const char *text, slist_t *rest);

/** 
 * Free the given string list.
 *
 * @param sl List of strings to free.
 */
void slist_free(slist_t *sl);

int slist_size(slist_t *sl);

// Allocates a new subset slist until the end index exclusive.
slist_t *slist_copy(slist_t *sl, int end);

void slist_print(slist_t *sl, const char *delim);

/**
 * Split the given on the given delimiter into a list of strings.
 *
 * Note, that the delimiter will not be included in any of the strings.
 *
 * @param text String to be split
 * @param delim A single character to use as the delimiter.
 *
 * @return a list containing all the substrings
 */
slist_t *slist_explode(const char *text, char delim);

#endif
