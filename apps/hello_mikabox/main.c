#include "mikabox.h"
#include <math.h>
#define M_PI  3.1415926535897932384626433832795

static uint64_t buttons;

static float x = 400, y = 240;
static float size = 90;

static float bounce = 0;

void draw()
{
  int ctx = gfx_ctx_create();

  int va = gfx_varr_create(6, 3);
  int ua = gfx_uarr_create(2);
  int sh = gfx_shad_create("#C");
  int bat = gfx_bat_create(va, ua, sh);

  while (1) {
    float p[3][5];
    p[0][0] = x - size;
    p[0][1] = y - size;
    p[0][2] = 1.0; p[0][3] = 0.6; p[0][4] = 0.6;

    p[1][0] = x + size;
    p[1][1] = y - size;
    p[1][2] = 0.6; p[1][3] = 1.0; p[1][4] = 0.6;

    p[2][0] = x - size;
    p[2][1] = y + size;
    p[2][2] = 0.6; p[2][3] = 0.6; p[2][4] = 1.0;

    gfx_varr_put(va, 0, p, 3);

    p[0][0] = x + size;
    p[0][1] = y + size;
    p[0][2] = 1.0; p[0][3] = 1.0; p[0][4] = 0.6;

    gfx_varr_put(va, 3, p, 3);

    unsigned background = 0xffffeecc;
    if (buttons != 0) background = 0xffffccee;
    gfx_ctx_config(ctx, gfx_tex_screen(), background);

    gfx_ctx_reset(ctx);
    gfx_ctx_batch(ctx, bat);
    gfx_ctx_call(ctx, 0, 6, 0);

    gfx_ctx_issue(ctx);
    mika_yield(1);
    gfx_ctx_wait(ctx);
  }
}

void synth()
{
  int block = aud_blocksize();
  float decay = expf(-(1.0f / 44100) * 3);

  short buf[block][2];

  while (1) {
    if (bounce) {
      float x = expf(-(bounce / 44100.0f) * 3) * 16384 * 0.3;
      for (int i = 0; i < block; i++) {
        buf[i][0] = buf[i][1] =
          sinf((bounce + i) / 44100.0f * 440 * M_PI * 2) * x;
        x *= decay;
      }
      bounce += block;
      aud_write(buf);
    }
    mika_yield(1);
  }
}

void event()
{
  buttons = 0;
  while (1) {
    buttons = mika_btns(0);
    mika_yield(1);
  }
}

void update()
{
  float vx = 0, vy = 0;
  uint64_t x_ban = 0, y_ban = 0;
  uint64_t t = mika_tick();

  while (1) {
    uint64_t t1 = mika_tick();
    float dt = (t1 - t) / 1e6f;
    t = t1;

    if (buttons & BTN_A) size -= 90 * dt;
    if (buttons & BTN_B) size += 90 * dt;
    if (size < 30) size = 30;
    if (size > 180) size = 180;

    float v_max = 43200 / size;
    float ux = 0, uy = 0;
    if (buttons & BTN_L) ux -= v_max;
    if (buttons & BTN_R) ux += v_max;
    if (buttons & BTN_U) uy -= v_max;
    if (buttons & BTN_D) uy += v_max;
    if (t1 < x_ban) ux = 0;
    if (t1 < y_ban) uy = 0;
    if (ux != 0 && uy != 0) {
      ux /= sqrtf(2);
      uy /= sqrtf(2);
    }

    float ul = ux * ux + uy * uy;
    float vl = vx * vx + vy * vy;
    float dx = ux - vx;
    float dy = uy - vy;
    float rate = (ul > vl ? 10 : 5) * dt;

    float x1 = x + (vx + dx * rate * 0.5) * dt;
    float y1 = y + (vy + dy * rate * 0.5) * dt;
    vx += dx * rate;
    vy += dy * rate;

    if (x1 < size) {
      x = size;
      vx = fabs(vx);
      x_ban = t1 + 20000;
      bounce = 1;
    } else if (x1 > 800 - size) {
      x = 800 - size;
      vx = -fabs(vx);
      x_ban = t1 + 20000;
      bounce = 1;
    } else {
      x = x1;
    }

    if (y1 < size) {
      y = size;
      vy = fabs(vy);
      y_ban = t1 + 20000;
      bounce = 1;
    } else if (y1 > 480 - size) {
      y = 480 - size;
      vy = -fabs(vy);
      y_ban = t1 + 20000;
      bounce = 1;
    } else {
      y = y1;
    }

    mika_yield(1);
  }
}

void main()
{
  mika_rout(draw, synth, event, update);
  mika_yield(1);
}
