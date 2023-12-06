///
/// General utilities
///

#include <stdio.h>

void repeat_print(const char *str, size_t n)
{
  for (size_t i = 0; i < n; i++)
  {
    printf("%s", str);
  }
}
