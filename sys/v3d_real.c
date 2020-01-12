#include "v3d.c.h"


struct v3d_mem v3d_mem_create(uint32_t size, uint32_t align, uint32_t flags)
{
  v3d_mem m;
  m.handle = gpumem_alloc(size, align, flags);
  m.addr = gpumem_lock(m.handle);
  return m;
}

#define _armptr(__mem)            ((uint8_t *)((__mem).addr & ~GPU_BUS_ADDR))
#define _offset(__armptr, __mem)  ((uint8_t *)(__armptr) - _armptr(__mem))
#define _align(__x, __align)      ((__x) = (((__x) + (__align) - 1) & ~((__align) - 1)))

void v3d_mem_lock(struct v3d_mem *m)
{
  m->addr = gpumem_lock(m->handle);
}

void v3d_mem_unlock(struct v3d_mem *m)
{
  m->addr = 0;
  gpumem_unlock(m->handle);
}

void v3d_mem_close(struct v3d_mem *m)
{
  gpumem_release(m->handle);
}

v3d_tex v3d_tex_screen(uint32_t buf)
{
  v3d_tex t = {
    .w = 800,
    .h = 480,
    .mem = {
      .handle = 0xfbfbfbfb,
      .addr = buf
    }
  };
  return t;
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

void v3d_ctx_anew(struct v3d_ctx *c, v3d_tex target)
{
  c->target = target;
  uint16_t w = target.w, h = target.h;
  const uint16_t bin_sidelen = 32;
  uint8_t bin_cols = (w + bin_sidelen - 1) / bin_sidelen;
  uint8_t bin_rows = (h + bin_sidelen - 1) / bin_sidelen;

  uint8_t *p;
  c->offs = 0;

  // Render control

  _align(c->offs, 0x1000);
  p = _armptr(c->mem) + c->offs;
  c->ren_ctrl_start = c->mem.addr + c->offs;

  // Clear Colours
  _putu8(&p, 114);
  _putu32(&p, 0xffafcfef);
  _putu32(&p, 0xffafcfef);
  _putu32(&p, 0);
  _putu8(&p, 0);

  // Tile Rendering Mode Configuration
  _putu8(&p, 113);
  _putu32(&p, target.mem.addr);
  _putu16(&p, w);
  _putu16(&p, h);
  _putu8(&p, (1 << 2) | (1 << 0));
  _putu8(&p, 0);

  // Tile Coordinates
  _putu8(&p, 115);
  _putu8(&p, 0);
  _putu8(&p, 0);

  // Store Tile Buffer General
  _putu8(&p, 28);
  _putu16(&p, 0);
  _putu32(&p, 0);

  for (uint8_t x = 0; x < bin_cols; x++)
  for (uint8_t y = 0; y < bin_rows; y++) {
    // Tile Coordinates
    _putu8(&p, 115);
    _putu8(&p, x);
    _putu8(&p, y);

    // Branch to Sub-list
    _putu8(&p, 17);
    _putu32(&p, c->mem.addr + 0x240000 + (y * bin_cols + x) * 32);

    // Store Multi-sample Resolved Tile Color Buffer
    // (and signal end of frame)
    _putu8(&p, (x == bin_cols - 1 && y == bin_rows - 1) ? 25 : 24);
  }

  c->offs = _offset(p, c->mem);
  c->ren_ctrl_end = c->mem.addr + c->offs;

  // Binning configuration

  _align(c->offs, 0x1000);
  p = _armptr(c->mem) + c->offs;
  c->bin_ctrl_start = c->mem.addr + c->offs;

  // Tile Binning Mode Configuration
  _putu8(&p, 112);
  _putu32(&p, c->mem.addr + 0x240000);
  _putu32(&p, 0x40000);
  _putu32(&p, c->mem.addr + 0x200000);
  _putu8(&p, bin_cols);
  _putu8(&p, bin_rows);
  _putu8(&p, (1 << 2) | (1 << 0));

  // Start Tile Binning
  _putu8(&p, 6);

  // Primitive List Format
  _putu8(&p, 56);
  _putu8(&p, 0x32);

  // Clip Window
  _putu8(&p, 102);
  _putu16(&p, 0);
  _putu16(&p, 0);
  _putu16(&p, w);
  _putu16(&p, h);

  // Configuration Bits
  _putu8(&p, 96);
  _putu8(&p, 3 | (1 << 6));
  _putu8(&p, 0x0);
  _putu8(&p, 0x2);

  // Viewport Offset
  _putu8(&p, 103);
  _putu16(&p, 0);
  _putu16(&p, 0);

  // Flush All State
  _putu8(&p, 5);
  // NOP
  _putu8(&p, 1);
  // Halt
  _putu8(&p, 0);

  c->offs = _offset(p, c->mem);
  c->bin_ctrl_end = c->mem.addr + c->offs;
}

void v3d_ctx_use_batch(struct v3d_ctx *c, const struct v3d_batch *batch)
{
}

void v3d_ctx_add_call(struct v3d_ctx *c, const struct v3d_call *call)
{
}

void v3d_ctx_issue(struct v3d_ctx *c)
{
  *V3D_L2CACTL = 4;
  *V3D_SLCACTL = 0x0f0f0f0f;

  *V3D_CT0CS = 0x20;
  while (*V3D_CT0CS & 0x20) { }
  *V3D_BFC = 1;
  *V3D_CT0CA = c->bin_ctrl_start;
  *V3D_CT0EA = c->bin_ctrl_end;
  while (*V3D_BFC == 0) { }

  *V3D_CT1CS = 0x20;
  while (*V3D_CT1CS & 0x20) { }
  *V3D_RFC = 1;
  *V3D_CT1CA = c->ren_ctrl_start;
  *V3D_CT1EA = c->ren_ctrl_end;
  while (*V3D_RFC == 0) { }
}
