#include "v3d.h"

static v3d_cty cty;
static v3d_ctx ctx;

void doda()
{
  v3d_init();
  //v3d_cty_init(&cty, 800, 480, 0);

  ctx = v3d_ctx_create();
}

void dodo(uint32_t fb)
{
  //cty.bufaddr = (uint32_t)fb;
  //v3d_op(&cty);
}
