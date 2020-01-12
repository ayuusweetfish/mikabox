#include "v3d.c.h"

struct v3d_mem v3d_mem_create(uint32_t size, uint32_t align, uint32_t flags)
{
  v3d_mem m;
  m.handle = gpumem_alloc(size, align, flags);
  m.addr = gpumem_lock(m.handle);
  return m;
}

void v3d_mem_lock(struct v3d_mem *m)
{
  m.addr = gpumem_lock(m.handle);
}

void v3d_mem_unlock(struct v3d_mem *m)
{
  m.addr = 0;
  gpumem_unlock(m->handle);
}

void v3d_mem_close(struct v3d_mem *m)
{
  gpumem_release(m->handle);
}

v3d_tex v3d_tex_screen(uint32_t buf)
{
}

v3d_tex v3d_tex_create(uint16_t w, uint16_t h, uint8_t *buf)
{
}

struct v3d_vertarr v3d_vertarr_create(uint16_t num, uint8_t num_varyings)
{
}

void v3d_vertarr_put(
  struct v3d_vertarr *a, uint32_t start_index,
  const v3d_vert *verts, uint32_t num
) {
}

struct v3d_unifarr v3d_unifarr_create(uint8_t num)
{
}

void v3d_unifarr_putu32(struct v3d_unifarr *a, uint32_t index, uint32_t value)
{
}

void v3d_unifarr_putf32(struct v3d_unifarr *a, uint32_t index, float value)
{
}

void v3d_unifarr_puttex(struct v3d_unifarr *a, uint32_t index, v3d_tex tex)
{
}

v3d_shader v3d_shader_create(const char *code)
{
}

struct v3d_ctx v3d_ctx_create()
{
  v3d_ctx c;
  c.mem = v3d_mem_create(0x280000, 0x1000, MEM_FLAG_COHERENT | MEM_FLAG_ZERO);
  c.offs = 0;
  return c;
}

void v3d_ctx_use_cfg(struct v3d_ctx *c, const struct v3d_cfg *cfg)
{
}

void v3d_ctx_use_batch(struct v3d_ctx *c, const struct v3d_batch *batch)
{
}

void v3d_ctx_add_call(struct v3d_ctx *c, const struct v3d_call *call)
{
}

void v3d_ctx_issue(struct v3d_ctx *c)
{
}
