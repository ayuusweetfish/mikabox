#include "mikabox.h"

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

  syscall(7, "Hello world!\n");

  char s[17] = { 0 };
  for (int i = 0; i < 5; i++) {
    uint64_t rnd = syscall64(6);
    for (int j = 15; j >= 0; j--) {
      s[j] = "0123456789abcdef"[rnd & 0xf];
      rnd >>= 1;
    }
    syscall(7, s);
  }

  return 0;
}
