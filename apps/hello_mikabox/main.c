#include "mikabox.h"
#include <math.h>
#include <stdbool.h>
#define M_PI  3.1415926535897932384626433832795

#define SCR_W 800
#define SCR_H 480

// Current buttons
static long long buttons;

// Position and size
static float x = 400, y = 240;
static float size = 90;

// 0: not bounced
// Positive: time since last bounce, in audio samples
static float bounce = 0;

static inline bool update_bounce(float *pos, float scr_size, float *vel)
{
  if (*pos < size) {
    *pos = size;
    *vel = fabs(*vel);
    return true;
  } else if (*pos > scr_size - size) {
    *pos = scr_size - size;
    *vel = -fabs(*vel);
    return true;
  }
  return false;
}

void draw()
{
  int ctx = gfx_ctx_create();

  int va = gfx_varr_create(6, 3);
  int ua = gfx_uarr_create(0);
  int sh = gfx_shad_create("#C");
  int bat = gfx_bat_create(va, ua, sh);

  while (1) {
    // A triangle's 5 parameters: X, Y, R, G, B
    float p[3][5];

    // The first triangle
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

    // The second triangle
    p[0][0] = x + size;
    p[0][1] = y + size;
    p[0][2] = 1.0; p[0][3] = 1.0; p[0][4] = 0.6;
    // The other two vertices are kept the same

    gfx_varr_put(va, 3, p, 3);

    // Draw call
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
          sinf((bounce + i) / 44100.0f * 442 * M_PI * 2) * x;
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
  long long x_ban = 0, y_ban = 0;
  long long t = mika_tick();

  const int ban_time = 20000; // milliseconds

  while (1) {
    long long t1 = mika_tick();
    float dt = (t1 - t) / 1e6f;
    t = t1;

    // Size change
    if (buttons & BTN_A) size -= 90 * dt;
    if (buttons & BTN_B) size += 90 * dt;
    if (size < 30) size = 30;
    if (size > 180) size = 180;

    // Desired velocity
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

    // Velocity change
    float ul = ux * ux + uy * uy;
    float vl = vx * vx + vy * vy;
    float rate = (ul > vl ? 10 : 5) * dt;
    float dx = (ux - vx) * rate;
    float dy = (uy - vy) * rate;

    // Position update
    x += (vx + dx * 0.5) * dt;
    y += (vy + dy * 0.5) * dt;
    vx += dx;
    vy += dy;

    // Bounces
    if (update_bounce(&x, SCR_W, &vx)) {
      x_ban = t1 + ban_time;
      bounce = 1;
    }
    if (update_bounce(&y, SCR_H, &vy)) {
      y_ban = t1 + ban_time;
      bounce = 1;
    }

    mika_yield(1);
  }
}

void main()
{
  mika_rout(draw, synth, event, update);
  mika_yield(1);
}
