///
/// General utilities
///

#ifndef _UTIL_H
#define _UTIL_H

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define TRUE  1
#define FALSE 0

typedef unsigned char byte_t;
typedef char bool_t;

void repeat_print(const char *str, size_t n);

#endif