#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mikabox.h"

// int __errno;
#undef printf
int printf(const char *format, ...)
{
  char s[256];
  va_list arg;
  va_start(arg, format);
  vsnprintf(s, sizeof s, format, arg);
  va_end(arg);
  mika_log(0, s);
}
// void sprintf() { }
// int vsprintf(char *str, const char *format, va_list ap)
// void memset() { }
// void memcmp() { }
// void memcpy() { }
// void strlen() { }
// void strncmp() { }
// void strtod() { }
// void strtoll() { }
clock_t clock() { return 0; }
void *realloc(void *ptr, size_t size)
{
  static uint32_t a[1 << 20];
  static uint32_t c = 0;

  void *ret = &a[c];
  c += (size + 3) / 4;
  //char s[256];
  //snprintf(s, 256, "%010p %3zd - %p / %p\n", ptr, size, ret, (char *)a + sizeof a);
  //mika_log(0, s);
  if (ptr != 0) memcpy(ret, ptr, size);
  return ret;
}
void free(void *ptr)
{
}
// void __aeabi_l2d() { }
// void __aeabi_uidivmod() { }
