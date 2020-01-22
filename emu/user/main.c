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
  register uint32_t r1 __asm__ ("r1") = 0;
  uint32_t j = 0;
  while (1) {
    for (uint32_t i = 0; i < 5e7; i++) {
      r1 += i + (++j);
      qwq[r1 % 1024] += r1;
    }
    register uint32_t r0 __asm__ ("r0") = (uint32_t)qvq;
    __asm__ __volatile__ (
      "mov r7, #43\n"
      "swi #0\n"
      "mov r7, #6\n"
      "swi #0\n"
      "swi #0\n"
      : "+r" (r0), "+r" (r1) :: "r7"
    );
  }
  return 251;
}
