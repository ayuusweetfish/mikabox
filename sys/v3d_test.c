#include "v3d.h"

#define STILL_DRAW_CTY 0
#if STILL_DRAW_CTY
static v3d_cty cty;
#endif

static v3d_ctx ctx;
static v3d_batch batch;

void doda()
{
  v3d_init();
#if STILL_DRAW_CTY
  v3d_cty_init(&cty, 800, 480, 0);
#endif

  ctx = v3d_ctx_create();

  v3d_vertarr va = v3d_vertarr_create(3, 0);
  v3d_vertarr_put(&va, 0, &(v3d_vert){2, 3}, 1);
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

  v3d_ctx_issue(&ctx);
}
