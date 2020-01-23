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

void draw()
{
  while (1) {
    for (int i = 0; i < 1e5; i++) __asm__ __volatile__ ("");
    syscall(7, "Hi > <");
    syscall(1);
  }
}

void synth()
{
  while (1) {
    syscall(1);
  }
}

void update()
{
  char s[17] = { 0 };
  while (1) {
    uint64_t rnd;
    for (int i = 0; i < 1e5; i++) {
      __asm__ __volatile__ ("");
      rnd = syscall64(6);
    }
    syscall(1);
    for (int j = 15; j >= 0; j--) {
      s[j] = "0123456789abcdef"[rnd & 0xf];
      rnd >>= 1;
    }
    syscall(7, s);
  }
}

void main()
{
  crt_init();

  syscall(7, "Hello world!\n");
  syscall(0, draw, synth, update);
  syscall(1);
}
