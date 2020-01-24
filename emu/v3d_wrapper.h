#ifndef _Mikabox_v3d_wrapper_h_
#define _Mikabox_v3d_wrapper_h_

#include <stdbool.h>
#include <stdint.h>

#define GLEW_STATIC
#include "GL/glew.h"

void v3d_init();

// Texture

typedef struct v3d_tex {
  uint16_t w, h;
  GLuint id;
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
v3d_tex v3d_tex_create(uint16_t w, uint16_t h);
void v3d_tex_update(v3d_tex *tex, uint8_t *buf, v3d_tex_fmt_t fmt);
void v3d_tex_close(v3d_tex *tex);

// Vertex and vertex array

typedef struct v3d_vert {
  float x, y;
  float varyings[];
} v3d_vert;

typedef struct v3d_vertarr {
  uint16_t num;
  uint8_t num_varyings;
  GLuint vbo_id;
} v3d_vertarr;

v3d_vertarr v3d_vertarr_create(uint16_t num, uint8_t num_varyings);
void v3d_vertarr_put(
  v3d_vertarr *a, uint32_t start_index,
  uint32_t verts, uint32_t num
);
void v3d_vertarr_close(v3d_vertarr *a);

// Uniform array

typedef struct v3d_unifarr {
  uint8_t num;
} v3d_unifarr;

v3d_unifarr v3d_unifarr_create(uint8_t num);
void v3d_unifarr_putu32(v3d_unifarr *a, uint32_t index, uint32_t value);
void v3d_unifarr_putf32(v3d_unifarr *a, uint32_t index, float value);
void v3d_unifarr_puttex(v3d_unifarr *a, uint32_t index, v3d_tex tex, uint8_t cfg);
void v3d_unifarr_close(v3d_unifarr *a);
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
  GLuint vsid;
  GLuint fsid;
} v3d_shader;

v3d_shader v3d_shader_create(const char *code);
void v3d_shader_close(v3d_shader *s);

// Batch (vertex array + uniform array + shader)

typedef struct v3d_batch {
  v3d_unifarr unifarr;
  GLuint vao_id;
  GLuint prog_id;
} v3d_batch;

v3d_batch v3d_batch_create(
  const v3d_vertarr vertarr,
  const v3d_unifarr unifarr,
  const v3d_shader shader
);
void v3d_batch_close(v3d_batch *b);

// Element (index) buffer

typedef struct v3d_buf {
  GLuint id;
} v3d_buf;

v3d_buf v3d_idxbuf_create(uint32_t count);
void v3d_idxbuf_copy(v3d_buf *m, uint32_t pos, uint32_t ptr, uint32_t count);
void v3d_idxbuf_close(v3d_buf *m);

// Draw call (batch + vertex indices)

typedef struct v3d_call {
  bool is_indexed;
  uint32_t num_verts;
  union {
    uint16_t start_index; // For plain arrays
    v3d_buf indices;      // For indexed arrays
  };
} v3d_call;

// Configuration (render target)

typedef struct v3d_ctx {
  v3d_tex target;
} v3d_ctx;

v3d_ctx v3d_ctx_create();
void v3d_ctx_anew(v3d_ctx *c, v3d_tex target, uint32_t clear);
void v3d_ctx_use_batch(v3d_ctx *c, const v3d_batch *batch);
void v3d_ctx_add_call(v3d_ctx *c, const v3d_call *call);
void v3d_ctx_issue(v3d_ctx *c);
void v3d_ctx_wait(v3d_ctx *c);
void v3d_ctx_close(v3d_ctx *c);

#endif
