#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "mikabox.h"

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

clock_t clock()
{
  return mika_wall() * CLOCKS_PER_SEC / 1000000;
}

extern uint8_t _initial_brk;
void *_sbrk(intptr_t increment)
{
  static intptr_t brkdiff = 0;
  void *ret = &_initial_brk + brkdiff;
  if ((brkdiff += increment) < 0) brkdiff = 0;
  return ret;
}

void _start()
{
  extern unsigned char _bss_begin;
  extern unsigned char _bss_end;
  void main();

  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
  main();
}

#define syscall_impl(_n, _r7_stmt) \
 ".global syscall" #_n "\n" \
 "syscall" #_n ":       \n" \
 "  push  {r7, lr}      \n" \
    _r7_stmt           "\n" \
 "  swi   #0            \n" \
 "  pop   {r7, pc}      \n"

__asm__ (
  syscall_impl(0, "mov r7, r0")
  syscall_impl(1, "mov r7, r1")
  syscall_impl(2, "mov r7, r2")
  syscall_impl(3, "mov r7, r3")
  syscall_impl(4, "ldr r7, [sp, #8]")
);
