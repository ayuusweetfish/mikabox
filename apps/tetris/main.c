#include "mikabox.h"
#include <math.h>
#include <stdio.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

__attribute__ ((noinline)) void crt_init()
{
  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
}

void draw()
{
  while (1) {
    mika_yield(1);
  }
}

void synth()
{
  while (1) {
    int drop = aud_dropped();
    if (drop)
      mika_printf("%d frame%s of audio dropped",
        drop, drop == 1 ? "" : "s");
    mika_yield(1);
  }
}

void event()
{
  while (1) {
    mika_yield(1);
  }
}

void update()
{
  while (1) {
    mika_yield(1);
  }
}

void main()
{
  crt_init();

  syscall(0, draw, synth, event, update);
  mika_yield(1);
}
