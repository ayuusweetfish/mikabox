#include "v3d.h"
#include "prop_tag.h"
#include <math.h>

#define STILL_DRAW_CTY 0
#if STILL_DRAW_CTY
static v3d_cty cty;
#endif

static v3d_ctx ctx;
static v3d_vertarr va;
static v3d_unifarr ua;
static v3d_batch batch;
static v3d_mem idxs;
static v3d_tex nanikore;

void doda()
{
  v3d_init();
#if STILL_DRAW_CTY
  v3d_cty_init(&cty, 800, 480, 0);
#endif

  ctx = v3d_ctx_create();

  va = v3d_vertarr_create(4, 5);
  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0} };
  v.x = 100.0f; v.y = 100.0f;
  v.varyings[0] = 0.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 1.0f;
  v.varyings[3] = 1.0f;
  v.varyings[4] = 1.0f;
  v3d_vertarr_put(&va, 0, &v, 1);
  v.x = 100.0f; v.y = 400.0f;
  v.varyings[0] = 0.0f;
  v.varyings[1] = 0.0f;
  v.varyings[2] = 0.5f;
  v.varyings[3] = 1.0f;
  v.varyings[4] = 1.0f;
  v3d_vertarr_put(&va, 1, &v, 1);
  v.x = 400.0f; v.y = 400.0f;
  v.varyings[0] = 1.0f;
  v.varyings[1] = 0.0f;
  v.varyings[2] = 1.0f;
  v.varyings[3] = 0.5f;
  v.varyings[4] = 1.0f;
  v3d_vertarr_put(&va, 2, &v, 1);
  v.x = 400.0f; v.y = 100.0f;
  v.varyings[0] = 1.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 1.0f;
  v.varyings[3] = 1.0f;
  v.varyings[4] = 0.5f;
  v3d_vertarr_put(&va, 3, &v, 1);

  ua = v3d_unifarr_create(3);

  batch = v3d_batch_create(va, ua, v3d_shader_create(""));

  idxs = v3d_mem_create(256, 128, MEM_FLAG_COHERENT);

  extern uint8_t _binary_utils_nanikore_bin_start;
  nanikore = v3d_tex_create(512, 256, &_binary_utils_nanikore_bin_start);
}

void dodo(uint32_t fb)
{
#if STILL_DRAW_CTY
  cty.bufaddr = (uint32_t)fb;
  v3d_op(&cty);
#endif

  v3d_ctx_anew(&ctx, v3d_tex_screen(fb));

  v3d_ctx_use_batch(&ctx, &batch);

  v3d_call call = {
    .is_indexed = false,
    .num_verts = 3,
    .start_index = 0,
  };
  v3d_ctx_add_call(&ctx, &call);

  uint16_t i[3] = {1, 2, 3};
  v3d_mem_copy(&idxs, 0, i, sizeof i);
  v3d_ctx_add_call(&ctx, &(v3d_call){
    .is_indexed = true,
    .num_verts = 3,
    .indices = idxs
  });

  // Any change before issuing will apply
  static uint32_t count = 0;
  float angle = (float)(++count) / 180.0f * acosf(-1.0f);
  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0} };
  v.x = 400.0f + 20.0f * cosf(angle);
  v.y = 400.0f + 20.0f * sinf(angle);
  v.varyings[0] = 1.0f;
  v.varyings[1] = 0.0f;
  v.varyings[2] = 1.0f;
  v.varyings[3] = 0.5f;
  v.varyings[4] = 1.0f;
  v3d_vertarr_put(&va, 2, &v, 1);

  v3d_unifarr_puttex(&ua, 0, nanikore);
  v3d_unifarr_putf32(&ua, 2, sinf(angle) * 0.5f + 0.5f);

  v3d_ctx_issue(&ctx);
}
