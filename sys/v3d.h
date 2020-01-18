#ifndef _Mikabox_v3d_h_
#define _Mikabox_v3d_h_

#include <stdbool.h>
#include <stdint.h>

void v3d_init();

// GPU memory region

typedef struct v3d_mem {
  uint32_t handle;
  uint32_t addr;  // Bus address
} v3d_mem;

struct v3d_mem v3d_mem_create(uint32_t size, uint32_t align, uint32_t flags);
void v3d_mem_lock(struct v3d_mem *m);
void v3d_mem_unlock(struct v3d_mem *m);
void v3d_mem_close(struct v3d_mem *m);
void v3d_mem_copy(struct v3d_mem *m, uint32_t offs, void *ptr, uint32_t size);

#define v3d_close(__objptr) v3d_mem_close((__objptr)->m)

// Texture

typedef struct v3d_tex {
  uint16_t w, h;
  v3d_mem mem;    // handle = 0xfbfbfbfb denotes screen
} v3d_tex;

typedef enum v3d_tex_fmt_t {
  v3d_tex_fmt_rgb = 0,
  v3d_tex_fmt_bgr,
  v3d_tex_fmt_rgba,
  v3d_tex_fmt_bgra,
  v3d_tex_fmt_argb,
  v3d_tex_fmt_abgr,
} v3d_tex_fmt_t;

v3d_tex v3d_tex_screen(uint32_t buf);
v3d_tex v3d_tex_create(uint16_t w, uint16_t h, uint8_t *buf, v3d_tex_fmt_t fmt);

// Vertex and vertex array

typedef struct v3d_vert {
  float x, y;
  float varyings[];
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
void v3d_unifarr_puttex(struct v3d_unifarr *a, uint32_t index, v3d_tex tex, uint8_t cfg);
#define v3d_wrap_s_repeat   (0 << 0)
#define v3d_wrap_s_clamp    (1 << 0)
#define v3d_wrap_s_mirror   (2 << 0)
#define v3d_wrap_s_border   (3 << 0)
#define v3d_wrap_t_repeat   (0 << 2)
#define v3d_wrap_t_clamp    (1 << 2)
#define v3d_wrap_t_mirror   (2 << 2)
#define v3d_wrap_t_border   (3 << 2)
#define v3d_minfilt_linear  (0 << 4)
#define v3d_minfilt_nearest (1 << 4)
#define v3d_minfilt_n_mip_n (2 << 4)
#define v3d_minfilt_n_mip_l (3 << 4)
#define v3d_minfilt_l_mip_n (4 << 4)
#define v3d_minfilt_l_mip_l (5 << 4)
#define v3d_magfilt_linear  (0 << 7)
#define v3d_magfilt_nearest (1 << 7)

// Shader

typedef struct v3d_shader {
  v3d_mem mem;
  bool is_multithreaded;
} v3d_shader;

v3d_shader v3d_shader_create(const char *code);

// Batch (vertex array + uniform array + shader)

typedef struct v3d_batch {
  v3d_mem mem;
} v3d_batch;

v3d_batch v3d_batch_create(
  const v3d_vertarr vertarr,
  const v3d_unifarr unifarr,
  const v3d_shader shader
);

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

typedef struct v3d_ctx {
  v3d_tex target;

  // Control list buffer
  v3d_mem mem;
  uint32_t offs;

  uint32_t ren_ctrl_start;
  uint32_t ren_ctrl_end;
  uint32_t bin_ctrl_start;
  uint32_t bin_ctrl_end;
} v3d_ctx;

struct v3d_ctx v3d_ctx_create();
void v3d_ctx_anew(struct v3d_ctx *c, v3d_tex target, uint32_t clear);
void v3d_ctx_use_batch(struct v3d_ctx *c, const struct v3d_batch *batch);
void v3d_ctx_add_call(struct v3d_ctx *c, const struct v3d_call *call);
void v3d_ctx_issue(struct v3d_ctx *c);
void v3d_ctx_wait(struct v3d_ctx *c);

#endif
