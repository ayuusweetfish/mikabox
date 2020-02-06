#include "main.h"
#include "mikabox.h"
#include <math.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

void crt_init()
{
  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
}

void draw()
{
  int ctx = gfx_ctx_create();
  while (1) {
    gfx_ctx_reset(ctx, gfx_tex_screen(), 0xffb0c0ff);
    mika_yield(1);
  }
}

uint64_t last_btns;
uint64_t cur_btns;

void event()
{
  last_btns = cur_btns = 0;
  while (1) {
    last_btns = cur_btns;
    cur_btns = mika_btns(0);
    if (btnp(BTN_A)) synth_note(0, 440, 10, 0, 0.2, true);
    if (btnp(BTN_B)) { ovw_stop(); ovw_start("b.out"); }
    if (btnr(BTN_X)) ovw_resume();
    if (btnp(BTN_Y)) synth_note(3, 440, 0.1, 0, 0.4, false);
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
