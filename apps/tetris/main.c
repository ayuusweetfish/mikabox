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

void init_mikan();
void update_mikan();
void *draw_mikan();

void draw()
{
  int ctx = gfx_ctx_create();
  int tex = gfx_tex_create(416, 256);
  float tw = 400.0f / 416;
  float th = 240.0f / 256;

  int va = gfx_varr_create(6, 2);
  int ua = gfx_uarr_create(2);
  int sh = gfx_shad_create("#T");
  int bat = gfx_bat_create(va, ua, sh);

  for (int i = 0; i <= 1; i++) {
    float attr[4];
    attr[0] = i * 800; attr[1] = i * 480;
    attr[2] = i * tw; attr[3] = i * th;
    gfx_varr_put(va, i * 3 + 0, attr, 1);
    attr[0] = 800; attr[1] = 0;
    attr[2] = tw; attr[3] = 0;
    gfx_varr_put(va, i * 3 + 1, attr, 1);
    attr[0] = 0; attr[1] = 480;
    attr[2] = 0; attr[3] = th;
    gfx_varr_put(va, i * 3 + 2, attr, 1);
  }

  gfx_uarr_puttex(ua, 0, tex, v3d_minfilt_nearest | v3d_magfilt_nearest);

  gfx_ctx_reset(ctx);
  gfx_ctx_batch(ctx, bat);
  gfx_ctx_call(ctx, 0, 6, 0);

  while (1) {
    void *buf = draw_mikan();
    gfx_tex_update(tex, buf, v3d_tex_fmt_bgr);

    gfx_ctx_config(ctx, gfx_tex_screen(), 0xff000000);

    gfx_ctx_issue(ctx);

    mika_yield(1);
    gfx_ctx_wait(ctx);
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
  /*
    if (btnp(BTN_A)) synth_note(0, 440, 10, 0, 0.2, true);
    if (btnp(BTN_B)) synth_note(0, 440, 1, 0, 0.1, false);
    if (btnp(BTN_X)) synth_note(2, 110, 1, 0, 0.4, false);
    if (btnp(BTN_Y)) synth_note(3, 440, 0.1, 0, 0.4, false);
  */
    mika_yield(1);
  }
}

void update()
{
  init_mikan();
  int last_frame = mika_tick() * 60 / 1000000;
  while (1) {
    int cur_frame = mika_tick() * 60 / 1000000;
    for (; last_frame < cur_frame; last_frame++) update_mikan();
    mika_yield(1);
  }
}

void main()
{
  crt_init();

  mika_rout(draw, synth, event, update);
  mika_yield(1);
}
