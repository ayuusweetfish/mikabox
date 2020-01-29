#include "mikabox.h"
#include "syscalls.h"
#include <math.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

uint8_t qwq[1024];
uint8_t qvq[] = "=~=";
uint8_t quq = { 0 };

uint32_t audio_blocksize;

static uint32_t buttons;

__attribute__ ((noinline)) void crt_init()
{
  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
}

void draw()
{
  int ctx, va3, ua3, sca, batch3;

  ctx = gfx_ctx_create();

  typedef struct v3d_vert {
    float x, y;
    float varyings[];
  } v3d_vert;
  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0, 0} };

  va3 = gfx_varr_create(3, 4);
  v.x = 10.0f; v.y = 10.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 1.0f; v.varyings[2] = 1.0f; v.varyings[3] = 1.0f;
  gfx_varr_put(va3, 0, &v, 1);
  v.x = 10.0f; v.y = 470.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 0.25f; v.varyings[2] = 0.5f; v.varyings[3] = 0.5f;
  gfx_varr_put(va3, 1, &v, 1);
  v.x = 600.0f; v.y = 245.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 0.5f; v.varyings[2] = 0.25f; v.varyings[3] = 0.5f;
  gfx_varr_put(va3, 2, &v, 1);

  ua3 = gfx_uarr_create(1);
  sca = gfx_shad_create("#CA");
  batch3 = gfx_bat_create(va3, ua3, sca);

  while (1) {
    gfx_ctx_wait(ctx);

    gfx_ctx_reset(ctx, gfx_tex_screen(), 0xffadbecf);

    gfx_ctx_batch(ctx, batch3);
    gfx_ctx_call(ctx, 0, 3, 0);

    gfx_ctx_issue(ctx);
    mika_yield(1);
  }
}

void synth()
{
}

void event()
{
}

void update()
{
}

void main()
{
  crt_init();

  mika_rout(draw, synth, event, update);
  mika_yield(1);
}
