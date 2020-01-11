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
} v3d_ctx;

void v3d_init();
void v3d_ctx_init(v3d_ctx *ctx, uint32_t w, uint32_t h, void *bufaddr);
void v3d_op(v3d_ctx *ctx);

typedef struct v3d_tex {
  uint16_t w, h;  // Dimensions
  uint32_t buf;   // Bus address; 0 denotes screen
} v3d_tex;

typedef struct v3d_call {
  bool is_indexed;
  uint32_t num_verts;
  union {
    uint16_t start_index; // For plain arrays
    uint16_t *indices;    // For indexed arrays
  };
} v3d_call;

// Input vertex data format:
// x (f32), y (f32)
// varyings (i32/f32 * num_varyings)

typedef struct v3d_vert {
  float x, y;
  uint32_t varyings[];
} v3d_vert;

typedef struct v3d_state {
  uint8_t num_uniforms;
  uint8_t num_varyings;
  uint16_t fshader_size;

  uint32_t *verts;
  uint32_t *uniforms;
  uint32_t *fshader;

  uint32_t mem_handle;
  uint32_t verts_bus;
  uint32_t uniforms_bus;
  uint32_t fshader_bus;
} v3d_state;

void v3d_state_init(
  struct v3d_state *s,
  uint32_t num_verts, uint8_t num_uniforms, uint8_t num_varyings,
  uint32_t *fshader, uint16_t fshader_size
);
void v3d_state_verts(
  struct v3d_state *s, const v3d_vert *verts,
  uint32_t start_index, uint32_t num
);
void v3d_state_close(struct v3d_state *s);

typedef struct v3d_cfg {
  v3d_tex tex;
} v3d_cfg;

typedef struct v3d_ctx1 {
  struct v3d_cfg cfg;
  struct v3d_state state;
} v3d_ctx1;

void v3d_ctx1_init(struct v3d_ctx1 *c);
void v3d_ctx1_use_config(struct v3d_ctx1 *c, struct v3d_cfg *cfg);
void v3d_ctx1_use_state(struct v3d_ctx1 *c, struct v3d_state *state);
void v3d_ctx1_issue(struct v3d_ctx1 *c);
void v3d_ctx1_close(struct v3d_ctx1 *c);

#endif
