#ifndef _Mikabox_v3d_h_
#define _Mikabox_v3d_h_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t w, h;
  uint32_t bufaddr;

  uint32_t rhandle;
  uint32_t rbusaddr;
  uint32_t rarmaddr;

  uint32_t thandle;
  uint32_t tbusaddr;
  uint32_t tarmaddr;
} v3d_cty;

void v3d_init();
void v3d_cty_init(v3d_cty *ctx, uint32_t w, uint32_t h, void *bufaddr);
void v3d_op(v3d_cty *ctx);

// Real API starts here

#define GPU_BUS_ADDR  0x40000000  // TODO: Unify all occurrences

// GPU memory region

typedef struct v3d_mem {
  uint32_t handle;
  uint32_t addr;  // Bus address
} v3d_mem;

#define v3d_armptr(__mem) ((uint8_t *)((__mem).addr & GPU_BUS_ADDR))

struct v3d_mem v3d_mem_create(uint32_t size, uint32_t align, uint32_t flags);
void v3d_mem_lock(struct v3d_mem *m);
void v3d_mem_unlock(struct v3d_mem *m);
void v3d_mem_close(struct v3d_mem *m);

#define v3d_close(__objptr) v3d_mem_close((__objptr)->m)

// Texture

typedef struct v3d_tex {
  uint16_t w, h;  // w = h = 0 denotes screen
  v3d_mem mem;
} v3d_tex;

v3d_tex v3d_tex_screen(uint32_t buf);
v3d_tex v3d_tex_create(uint16_t w, uint16_t h, uint8_t *buf);

// Vertex and vertex array

typedef struct v3d_vert {
  float x, y;
  uint32_t varyings[];
} v3d_vert;

typedef struct v3d_vertarr {
  uint16_t num;
  uint8_t num_varyings;
  v3d_mem mem;
} v3d_vertarr;

struct v3d_vertarr v3d_vertarr_create(uint16_t num, uint8_t num_varyings);
void v3d_vertarr_put(
  struct v3d_vertarr *a, uint32_t start_index,
  const v3d_vert *verts, uint32_t num
);

// Uniform array

typedef struct v3d_unifarr {
  uint8_t num;
  v3d_mem mem;
} v3d_unifarr;

struct v3d_unifarr v3d_unifarr_create(uint8_t num);
void v3d_unifarr_putu32(struct v3d_unifarr *a, uint32_t index, uint32_t value);
void v3d_unifarr_putf32(struct v3d_unifarr *a, uint32_t index, float value);
void v3d_unifarr_puttex(struct v3d_unifarr *a, uint32_t index, v3d_tex tex);

// Shader

typedef struct v3d_shader {
  v3d_mem mem;
} v3d_shader;

v3d_shader v3d_shader_create(const char *code);

// Batch (vertex array + uniform array + shader)

typedef struct v3d_batch {
  v3d_vertarr vertarr;
  v3d_unifarr unifarr;
  v3d_shader shader;
} v3d_batch;

// Draw call (batch + vertex indices)

typedef struct v3d_call {
  bool is_indexed;
  uint32_t num_verts;
  union {
    uint16_t start_index; // For plain arrays
    v3d_mem indices;      // For indexed arrays
  };
} v3d_call;

// Configuration (render target)

typedef struct v3d_cfg {
  v3d_tex tex;
} v3d_cfg;

typedef struct v3d_ctx {
  // Control list buffer
  v3d_mem mem;
  uint32_t offs;
} v3d_ctx;

struct v3d_ctx v3d_ctx_create();
void v3d_ctx_use_cfg(struct v3d_ctx *c, const struct v3d_cfg *cfg);
void v3d_ctx_use_batch(struct v3d_ctx *c, const struct v3d_batch *batch);
void v3d_ctx_add_call(struct v3d_ctx *c, const struct v3d_call *call);
void v3d_ctx_issue(struct v3d_ctx *c);

#endif
