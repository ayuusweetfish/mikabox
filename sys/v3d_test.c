#include "v3d.h"
#include "prop_tag.h"
#include <math.h>

static v3d_ctx ctx;
static v3d_vertarr va1, va2, va3;
static v3d_unifarr ua1, ua2, ua3;
static v3d_batch batch1, batch2, batch3;
static v3d_mem idxs;
static v3d_tex nanikore, checker;
static v3d_tex target;

void doda()
{
  v3d_init();
  ctx = v3d_ctx_create();

  va1 = v3d_vertarr_create(2504, 6);
  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0, 0} };
  v.x = 250.0f; v.y = 100.0f;
  v.varyings[0] = 0.5f;
  v.varyings[1] = 0.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  v3d_vertarr_put(&va1, 0, &v, 1);
  v.x = 100.0f; v.y = 400.0f;
  v.varyings[0] = 0.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  v3d_vertarr_put(&va1, 1, &v, 1);
  v.x = 400.0f; v.y = 400.0f;
  v.varyings[0] = 1.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  v3d_vertarr_put(&va1, 2, &v, 1);
  v.x = 400.0f; v.y = 100.0f;
  v.varyings[0] = 1.0f;
  v.varyings[1] = 0.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  for (uint16_t i = 0; i < 50; i++)
  for (uint16_t j = 0; j < 50; j++) {
    v.x = 100.0f + 6.0f * i; v.y = 100.0f + 6.0f * j;
    v.varyings[0] = i * 0.02f;
    v.varyings[1] = j * 0.02f;
    v.varyings[2] = 0.3f;
    v.varyings[3] = 0.3f;
    v.varyings[4] = 0.3f;
    v.varyings[5] = 0.3f;
    v3d_vertarr_put(&va1, 4 + i * 50 + j, &v, 1);
  }

  ua1 = v3d_unifarr_create(3);

  batch1 = v3d_batch_create(va1, ua1, v3d_shader_create("#texture_chroma_alpha"));

  idxs = v3d_mem_create(25600, 128, MEM_FLAG_COHERENT);
  static uint16_t p[49][49][3];
  for (uint16_t i = 0; i < 49; i++)
  for (uint16_t j = 0; j < 49; j++) {
    p[i][j][0] = 4 + i * 50 + j;
    p[i][j][1] = 4 + i * 50 + j + 1;
    p[i][j][2] = 4 + i * 50 + j + 50;
  }
  p[0][0][0] = 0;
  p[0][0][1] = 1;
  p[0][0][2] = 2;
  p[0][1][0] = 4;
  p[0][1][1] = 14;
  p[0][1][2] = 504;
  v3d_mem_copy(&idxs, 0, p, sizeof p);

  extern uint8_t _binary_utils_nanikore_bin_start;
  nanikore = v3d_tex_create(512, 256, &_binary_utils_nanikore_bin_start);

  va2 = v3d_vertarr_create(6, 6);
  v.varyings[2] = 1.0;
  v.varyings[3] = 1.0;
  v.varyings[4] = 1.0;
  v.varyings[5] = 1.0;
  for (uint8_t i = 0; i <= 1; i++) {
    v.x = 40.0f + i * 720;
    v.y = 40.0f + i * 400;
    v.varyings[0] = i - 0.3f; v.varyings[1] = i;
    v3d_vertarr_put(&va2, i * 3 + 0, &v, 1);
    v.x = 760.0f; v.y = 40.0f; v.varyings[0] = 0.7f; v.varyings[1] = 0.0f;
    v3d_vertarr_put(&va2, i * 3 + 1, &v, 1);
    v.x = 40.0f; v.y = 440.0f; v.varyings[0] = -0.3f; v.varyings[1] = 1.0f;
    v3d_vertarr_put(&va2, i * 3 + 2, &v, 1);
  }

  ua2 = v3d_unifarr_create(2);
  batch2 = v3d_batch_create(va2, ua2, v3d_shader_create("#texture"));

#define cw 32
#define ch 64
  static uint8_t c[ch][cw][3];
  for (int i = 0; i < ch; i++)
    for (int j = 0; j < cw; j++)
      c[i][j][0] = c[i][j][1] = c[i][j][2] =
        ((i < 4 && j < 4) || ((i ^ j) & 4)) ? 0xff : 0xcc;

  checker = v3d_tex_create(cw, ch, &c[0][0][0]);

  target = v3d_tex_create(800, 480, NULL);
  v3d_unifarr_puttex(&ua2, 0, target, v3d_magfilt_nearest | v3d_wrap_s_mirror);

  va3 = v3d_vertarr_create(3, 4);
  v.x = 10.0f; v.y = 10.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 1.0f; v.varyings[2] = 1.0f; v.varyings[3] = 1.0f;
  v3d_vertarr_put(&va3, 0, &v, 1);
  v.x = 10.0f; v.y = 470.0f;
  v.varyings[0] = 1.0f; v.varyings[1] = 0.5f; v.varyings[2] = 1.0f; v.varyings[3] = 1.0f;
  v3d_vertarr_put(&va3, 1, &v, 1);
  v.x = 600.0f; v.y = 245.0f;
  v.varyings[0] = 1.0f; v.varyings[1] = 1.0f; v.varyings[2] = 0.5f; v.varyings[3] = 0.5f;
  v3d_vertarr_put(&va3, 2, &v, 1);

  ua3 = v3d_unifarr_create(1);
  v3d_unifarr_putf32(&ua3, 0, 0);
  batch3 = v3d_batch_create(va3, ua3, v3d_shader_create("#chroma_alpha"));
}

void dodo(uint32_t fb)
{
  // Render to texture
  v3d_ctx_wait(&ctx);
  v3d_ctx_anew(&ctx, v3d_tex_screen(fb), 0x0);

  v3d_ctx_use_batch(&ctx, &batch1);

  extern bool has_key;
  v3d_call call = {
    .is_indexed = true,
    .num_verts = 3 * (has_key ? 1000 : 1),
    .indices = idxs
  };
  v3d_ctx_add_call(&ctx, &call);

  // Any change before issuing will apply
  static uint32_t count = 0;
  float angle = (float)(++count) / 180.0f * acosf(-1.0f);
  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0, 0} };
  v.x = 400.0f + 20.0f * cosf(angle);
  v.y = 400.0f + 20.0f * sinf(angle);
  v.varyings[0] = 1.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  v3d_vertarr_put(&va1, 2, &v, 1);

  v3d_unifarr_puttex(&ua1, 0, nanikore, 0);
  v3d_unifarr_putf32(&ua1, 2, sinf(angle) * 0.5f + 0.5f);

  v3d_ctx_issue(&ctx);
/*
  // Render to screen
  v3d_ctx_wait(&ctx);
  v3d_ctx_anew(&ctx, v3d_tex_screen(fb), 0xff000000);

  v3d_ctx_use_batch(&ctx, &batch2);
  v3d_ctx_add_call(&ctx, &(v3d_call){
    .is_indexed = false,
    .num_verts = 6,
    .start_index = 0,
  });

  v3d_ctx_issue(&ctx);
*/
}
