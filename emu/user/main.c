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
  int ctx = syscall(256 + 0);
  while (1) {
    syscall(256 + 5, ctx);
    uint64_t t = syscall64(4, 0);
    syscall(256 + 1, ctx, syscall(256 + 18),
      t == 0 ? 0xffffffff : 0xffffddcc);

    syscall(256 + 4, ctx);
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
    syscall(1);
    uint64_t t = syscall64(4, 0);
    for (int j = 15; j >= 0; j--) {
      s[j] = "0123456789abcdef"[t & 0xf];
      t >>= 4;
    }
    syscall(7, s);
  }
}

void main()
{
  crt_init();

  syscall(7, "Hello world!\n");
  syscall(0, update, synth, draw);
  syscall(1);
}
