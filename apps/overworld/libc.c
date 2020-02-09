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
// void __aeabi_l2d() { }
// void __aeabi_uidivmod() { }

extern uint8_t _initial_brk;
void *_sbrk(intptr_t increment)
{
  static intptr_t brkdiff = 0;
  void *ret = &_initial_brk + brkdiff;
  if ((brkdiff += increment) < 0) brkdiff = 0;
  return ret;
}
