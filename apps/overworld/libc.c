#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
clock_t clock() { return mika_wall() * CLOCKS_PER_SEC / 1000000; }
extern uint8_t _initial_brk;
void *realloc(void *ptr, size_t size)
{
  static uint32_t a[1 << 20] __attribute__ ((aligned(8)));
  static uint32_t c = 0;

  uint32_t osize = (ptr == 0 ? 0 : *((uint32_t *)ptr - 1));
  if (size < osize) return ptr;

  a[c] = size;
  void *ret = &a[c + 1];
  c += (size + 3) / 4 + 1;
  c += (c & 1);
  //char s[256];
  //snprintf(s, 256, "%010p %3u - %p %3zd / %p\n",
  //  ptr, osize, ret, size, (char *)a + sizeof a);
  //mika_log(0, s);
  if (ptr != 0) memcpy(ret, ptr, osize);
  memset(ret + osize, 0, size - osize);
  return ret;
}
void *__wrap__malloc_r(void *_unused, size_t size)
{
  return realloc(0, size);
}
void __wrap__free_r(void *_unused, void *ptr)
{
}
// void __aeabi_l2d() { }
// void __aeabi_uidivmod() { }
