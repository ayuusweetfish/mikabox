#include "v3d.h"

static v3d_cty ctx;

void doda()
{
  v3d_init();
  v3d_cty_init(&ctx, 800, 480, 0);
}

void dodo(uint32_t fb)
{
  ctx.bufaddr = (uint32_t)fb;
  v3d_op(&ctx);
}
