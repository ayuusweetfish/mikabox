#ifndef _Mikabox_v3d_h_
#define _Mikabox_v3d_h_

#include <stdint.h>

typedef struct {
  uint32_t w, h;
  uint32_t bufaddr;

  uint32_t rhandle;
  uint32_t rbusaddr;
  uint32_t rarmaddr;
} v3d_ctx;

void v3d_init();
void v3d_ctx_init(v3d_ctx *ctx, uint32_t w, uint32_t h, void *bufaddr);

#endif
