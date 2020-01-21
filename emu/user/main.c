#include <stdint.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

uint8_t qwq[1024];
uint8_t qvq[] = "=~=";
uint8_t quq = { 0 };

__attribute__ ((noinline)) void crt_init()
{
  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
}

uint32_t main()
{
  crt_init();
  register uint32_t r0 __asm__ ("r0") = (uint32_t)qvq;
  register uint32_t r1 __asm__ ("r1") = 99;
  __asm__ __volatile__ ("swi #0" :: "r" (r0), "r" (r1));
  return 251;
}
