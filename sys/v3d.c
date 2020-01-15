#include "v3d.h"
#include "main.h"
#include "common.h"
#include "prop_tag.h"
#include "printf/printf.h"
#include "coroutine.h"

#include <math.h>
#include <string.h>

#define GPU_BUS_ADDR  0x40000000  // TODO: Unify all occurrences
#define v3d_printf(...)

#define v3d_reg(_offs) (volatile uint32_t *)(PERI_BASE + 0xc00000 + (_offs))

#define V3D_IDENT0  v3d_reg(0x000)  // V3D Identification 0 (V3D block identity)
#define V3D_IDENT1  v3d_reg(0x004)  // V3D Identification 1 (V3D Configuration A)
#define V3D_IDENT2  v3d_reg(0x008)  // V3D Identification 2 (V3D Configuration B)

#define V3D_SCRATCH v3d_reg(0x010)  // Scratch Register

#define V3D_L2CACTL v3d_reg(0x020)  // 2 Cache Control
#define V3D_SLCACTL v3d_reg(0x024)  // Slices Cache Control

#define V3D_INTCTL  v3d_reg(0x030)  // Interrupt Control
#define V3D_INTENA  v3d_reg(0x034)  // Interrupt Enables
#define V3D_INTDIS  v3d_reg(0x038)  // Interrupt Disables

#define V3D_CT0CS   v3d_reg(0x100)  // Control List Executor Thread 0 Control and Status.
#define V3D_CT1CS   v3d_reg(0x104)  // Control List Executor Thread 1 Control and Status.
#define V3D_CT0EA   v3d_reg(0x108)  // Control List Executor Thread 0 End Address.
#define V3D_CT1EA   v3d_reg(0x10c)  // Control List Executor Thread 1 End Address.
#define V3D_CT0CA   v3d_reg(0x110)  // Control List Executor Thread 0 Current Address.
#define V3D_CT1CA   v3d_reg(0x114)  // Control List Executor Thread 1 Current Address.
#define V3D_CT00RA0 v3d_reg(0x118)  // Control List Executor Thread 0 Return Address.
#define V3D_CT01RA0 v3d_reg(0x11c)  // Control List Executor Thread 1 Return Address.
#define V3D_CT0LC   v3d_reg(0x120)  // Control List Executor Thread 0 List Counter
#define V3D_CT1LC   v3d_reg(0x124)  // Control List Executor Thread 1 List Counter
#define V3D_CT0PC   v3d_reg(0x128)  // Control List Executor Thread 0 Primitive List Counter
#define V3D_CT1PC   v3d_reg(0x12c)  // Control List Executor Thread 1 Primitive List Counter

#define V3D_PCS     v3d_reg(0x130)  // V3D Pipeline Control and Status
#define V3D_BFC     v3d_reg(0x134)  // Binning Mode Flush Count
#define V3D_RFC     v3d_reg(0x138)  // Rendering Mode Frame Count

#define V3D_BPCA    v3d_reg(0x300)  // Current Address of Binning Memory Pool
#define V3D_BPCS    v3d_reg(0x304)  // Remaining Size of Binning Memory Pool
#define V3D_BPOA    v3d_reg(0x308)  // Address of Overspill Binning Memory Block
#define V3D_BPOS    v3d_reg(0x30c)  // Size of Overspill Binning Memory Block
#define V3D_BXCF    v3d_reg(0x310)  // Binner Debug

#define V3D_SQRSV0  v3d_reg(0x410)  // Reserve QPUs 0-7
#define V3D_SQRSV1  v3d_reg(0x414)  // Reserve QPUs 8-15
#define V3D_SQCNTL  v3d_reg(0x418)  // QPU Scheduler Control

#define V3D_SRQPC   v3d_reg(0x430)  // QPU User Program Request Program Address
#define V3D_SRQUA   v3d_reg(0x434)  // QPU User Program Request Uniforms Address
#define V3D_SRQUL   v3d_reg(0x438)  // QPU User Program Request Uniforms Length
#define V3D_SRQCS   v3d_reg(0x43c)  // QPU User Program Request Control and Status

#define V3D_VPACNTL v3d_reg(0x500)  // VPM Allocator Control
#define V3D_VPMBASE v3d_reg(0x504)  // VPM base (user  memory reservation

#define V3D_PCTRC   v3d_reg(0x670)  // Performance Counter Clear
#define V3D_PCTRE   v3d_reg(0x674)  // Performance Counter Enables

#define V3D_PCTR0   v3d_reg(0x680)  // Performance Counter Count 0
#define V3D_PCTRS0  v3d_reg(0x684)  // Performance Counter Mapping 0
#define V3D_PCTR1   v3d_reg(0x688)  // Performance Counter Count 1
#define V3D_PCTRS1  v3d_reg(0x68c)  // Performance Counter Mapping 1
#define V3D_PCTR2   v3d_reg(0x690)  // Performance Counter Count 2
#define V3D_PCTRS2  v3d_reg(0x694)  // Performance Counter Mapping 2
#define V3D_PCTR3   v3d_reg(0x698)  // Performance Counter Count 3
#define V3D_PCTRS3  v3d_reg(0x69c)  // Performance Counter Mapping 3
#define V3D_PCTR4   v3d_reg(0x6a0)  // Performance Counter Count 4
#define V3D_PCTRS4  v3d_reg(0x6a4)  // Performance Counter Mapping 4
#define V3D_PCTR5   v3d_reg(0x6a8)  // Performance Counter Count 5
#define V3D_PCTRS5  v3d_reg(0x6ac)  // Performance Counter Mapping 5
#define V3D_PCTR6   v3d_reg(0x6b0)  // Performance Counter Count 6
#define V3D_PCTRS6  v3d_reg(0x6b4)  // Performance Counter Mapping 6
#define V3D_PCTR7   v3d_reg(0x6b8)  // Performance Counter Count 7
#define V3D_PCTRS7  v3d_reg(0x6bc)  // Performance Counter Mapping 7 
#define V3D_PCTR8   v3d_reg(0x6c0)  // Performance Counter Count 8
#define V3D_PCTRS8  v3d_reg(0x6c4)  // Performance Counter Mapping 8
#define V3D_PCTR9   v3d_reg(0x6c8)  // Performance Counter Count 9
#define V3D_PCTRS9  v3d_reg(0x6cc)  // Performance Counter Mapping 9
#define V3D_PCTR10  v3d_reg(0x6d0)  // Performance Counter Count 10
#define V3D_PCTRS10 v3d_reg(0x6d4)  // Performance Counter Mapping 10
#define V3D_PCTR11  v3d_reg(0x6d8)  // Performance Counter Count 11
#define V3D_PCTRS11 v3d_reg(0x6dc)  // Performance Counter Mapping 11
#define V3D_PCTR12  v3d_reg(0x6e0)  // Performance Counter Count 12
#define V3D_PCTRS12 v3d_reg(0x6e4)  // Performance Counter Mapping 12
#define V3D_PCTR13  v3d_reg(0x6e8)  // Performance Counter Count 13
#define V3D_PCTRS13 v3d_reg(0x6ec)  // Performance Counter Mapping 13
#define V3D_PCTR14  v3d_reg(0x6f0)  // Performance Counter Count 14
#define V3D_PCTRS14 v3d_reg(0x6f4)  // Performance Counter Mapping 14
#define V3D_PCTR15  v3d_reg(0x6f8)  // Performance Counter Count 15
#define V3D_PCTRS15 v3d_reg(0x6fc)  // Performance Counter Mapping 15

#define V3D_DBGE    v3d_reg(0xf00)  // PSE Error Signals
#define V3D_FDBGO   v3d_reg(0xf04)  // FEP Overrun Error Signals
#define V3D_FDBGB   v3d_reg(0xf08)  // FEP Interface Ready and Stall Signals, FEP Busy Signals
#define V3D_FDBGR   v3d_reg(0xf0c)  // FEP Internal Ready Signals
#define V3D_FDBGS   v3d_reg(0xf10)  // FEP Internal Stall Input Signals

#define V3D_ERRSTAT v3d_reg(0xf20)  // Miscellaneous Error Signals (VPM, VDW, VCD, VCM, L2C)

// Make sure `float` is 32-bit
typedef char _ensure_float_32[sizeof(float) == 4 ? 1 : -1];

static inline void _putu8(uint8_t **p, uint8_t x)
{
  (*p)[0] = x; *p += 1;
}
static inline void _putu16(uint8_t **p, uint16_t x)
{
  uint8_t *q = (uint8_t *)&x;
  (*p)[0] = q[0]; (*p)[1] = q[1];
  *p += 2;
}
static inline void _putu32(uint8_t **p, uint32_t x)
{
  uint8_t *q = (uint8_t *)&x;
  (*p)[0] = q[0]; (*p)[1] = q[1]; (*p)[2] = q[2]; (*p)[3] = q[3];
  *p += 4;
}
static inline void _putf32(uint8_t **p, float x)
{
  uint8_t *q = (uint8_t *)&x;
  (*p)[0] = q[0]; (*p)[1] = q[1]; (*p)[2] = q[2]; (*p)[3] = q[3];
  *p += 4;
}

void v3d_init()
{
  set_clock_rate(5, 250 * 1000 * 1000);
  enable_qpu();
  if (*V3D_IDENT0 != 0x02443356) {
    v3d_printf("Cannot enable QPUs, what can go wrong?\n");
    return;
  }
  v3d_printf("QPUs enabled\n");
}

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
#define _count(__arr)             ((sizeof (__arr)) / (sizeof (__arr)[0]))

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

void v3d_mem_copy(struct v3d_mem *m, uint32_t offs, void *ptr, uint32_t size)
{
  void *p = _armptr(*m) + offs;
  memcpy(p, ptr, size);
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

#define is_screen(__tex)  ((__tex).mem.handle == 0xfbfbfbfb)

// x and y are coordinates of the top-left corner (left handed/y+ down)
// XXX: Consider using right handed coordinates instead?
static inline void microtile(
  uint32_t *tex, uint16_t w, uint16_t h, uint8_t *buf,
  uint16_t x, uint16_t y)
{
  for (uint16_t y3 = 0; y3 < 4; y3++)
  for (uint16_t x3 = 0; x3 < 4; x3++) {
    uint16_t x4 = x + x3;
    uint16_t y4 = y + y3;
    uint32_t p = ((uint32_t)y4 * w + x4) * 3;
    uint32_t value = ((uint32_t)buf[p] << 16) |
      ((uint32_t)buf[p + 1] << 8) | (uint32_t)buf[p + 2];
    *(tex++) = value;
  }
}

v3d_tex v3d_tex_create(uint16_t w, uint16_t h, uint8_t *buf)
{
  v3d_tex t;
  t.w = w;
  t.h = h;
  t.mem = v3d_mem_create((uint32_t)w * h * 4, 4096,
    MEM_FLAG_L1_NONALLOCATING | MEM_FLAG_ZERO | MEM_FLAG_HINT_PERMALOCK);

  if (buf == NULL) return t;

  uint32_t *tex = (uint32_t *)_armptr(t.mem);
  // 4K tiles
  for (uint16_t y0i = 0; y0i < h / 32; y0i++) {
    uint16_t y0 = y0i * 32;
    for (uint16_t x0i = 0; x0i < w / 32; x0i++) {
      uint16_t x0 = ((y0i & 1) ? w - 32 * (x0i + 1) : 32 * x0i);
      // Four 1K subtiles
      static const uint8_t subt[4] = {0, 1, 3, 2};
      for (uint8_t k = 0; k < 4; k++) {
        uint16_t x1 = x0 + ((subt[k] >> 1) ^ (y0i & 1)) * 16;
        uint16_t y1 = y0 + ((subt[k] & 1) ^ (y0i & 1)) * 16;
        // A subtile with top-left corner at (y1, x1)
        // Emit the subtile
        for (uint16_t y2 = 0; y2 < 16; y2 += 4)
        for (uint16_t x2 = 0; x2 < 16; x2 += 4) {
          // Microtile
          microtile(tex, w, h, buf, x1 + x2, y1 + y2);
          tex += 16;
        }
      }
    }
  }

  return t;
}

struct v3d_vertarr v3d_vertarr_create(uint16_t num, uint8_t num_varyings)
{
  v3d_vertarr a;
  a.num = num;
  a.num_varyings = num_varyings;
  size_t pstride = 12 + 4 * num_varyings;
  a.mem = v3d_mem_create(pstride * num, 128,
    MEM_FLAG_COHERENT | MEM_FLAG_ZERO | MEM_FLAG_HINT_PERMALOCK);
  return a;
}

void v3d_vertarr_put(
  struct v3d_vertarr *a, uint32_t start_index,
  const v3d_vert *verts, uint32_t num
) {
  uint32_t num_varyings = a->num_varyings;

  uint8_t *p = _armptr(a->mem);
  size_t pstride = 12 + 4 * num_varyings;
  p += pstride * start_index;

  const uint8_t *q = (const uint8_t *)verts;
  size_t qstride = sizeof(v3d_vert) + sizeof(float) * num_varyings;

  for (uint32_t i = 0; i < num; i++) {
    const v3d_vert *vert = (const v3d_vert *)q;
    _putu16(&p, (uint16_t)(vert->x * 16 + 0.5f));
    _putu16(&p, (uint16_t)(vert->y * 16 + 0.5f));
    _putf32(&p, 1.0f);
    _putf32(&p, 1.0f);
    for (uint32_t j = 0; j < num_varyings; j++)
      _putf32(&p, vert->varyings[j]);
    q += qstride;
  }
}

struct v3d_unifarr v3d_unifarr_create(uint8_t num)
{
  v3d_unifarr a;
  a.num = num;
  a.mem = v3d_mem_create(4 * num, 4,
    MEM_FLAG_COHERENT | MEM_FLAG_ZERO | MEM_FLAG_HINT_PERMALOCK);
  return a;
}

void v3d_unifarr_putu32(struct v3d_unifarr *a, uint32_t index, uint32_t value)
{
  uint32_t *p = (uint32_t *)_armptr(a->mem);
  p[index] = value;
}

void v3d_unifarr_putf32(struct v3d_unifarr *a, uint32_t index, float value)
{
  float *p = (float *)_armptr(a->mem);
  p[index] = value;
}

void v3d_unifarr_puttex(struct v3d_unifarr *a, uint32_t index, v3d_tex tex, uint8_t cfg)
{
  uint32_t *p = (uint32_t *)_armptr(a->mem);
  p[index] = tex.mem.addr;
  p[index + 1] = (tex.w << 8) | (tex.h << 20) | cfg;
}

static const uint32_t chroma_shader[] = {
  #include "v3d/shader_chroma.fx.h"
};

static const uint32_t chroma_alpha_shader[] = {
  #include "v3d/shader_chroma_alpha.fx.h"
};

static const uint32_t tex_shader[] = {
  #include "v3d/shader_tex.fx.h"
};

static const uint32_t tex_chroma_shader[] = {
  #include "v3d/shader_tex_chroma.fx.h"
};

v3d_shader v3d_shader_create(const char *code)
{
  const uint32_t *shader = NULL;
  uint32_t count = 0;
#define use(__shader) \
  do { shader = (__shader); count = _count(__shader); } while (0)

  if (strcmp(code, "#chroma") == 0)
    use(chroma_shader);
  else if (strcmp(code, "#chroma_alpha") == 0)
    use(chroma_alpha_shader);
  else if (strcmp(code, "#texture") == 0)
    use(tex_shader);
  else if (strcmp(code, "#texture_chroma") == 0)
    use(tex_chroma_shader);

#undef use
  v3d_shader s;
  s.mem = v3d_mem_create(count * 4, 8,
    MEM_FLAG_COHERENT | MEM_FLAG_ZERO | MEM_FLAG_HINT_PERMALOCK);
  uint8_t *p = _armptr(s.mem);

  for (uint32_t i = 0; i < count; i++) _putu32(&p, shader[i]);

  return s;
}

v3d_batch v3d_batch_create(
  const v3d_vertarr vertarr,
  const v3d_unifarr unifarr,
  const v3d_shader shader
) {
  v3d_batch b;
  b.mem = v3d_mem_create(16, 16,
    MEM_FLAG_COHERENT | MEM_FLAG_NO_INIT | MEM_FLAG_HINT_PERMALOCK);
  uint8_t *p = _armptr(b.mem);

  _putu8(&p, 1);
  _putu8(&p, 12 + 4 * vertarr.num_varyings);
  _putu8(&p, 0xcc);
  _putu8(&p, vertarr.num_varyings);
  _putu32(&p, shader.mem.addr);
  _putu32(&p, unifarr.mem.addr);
  _putu32(&p, vertarr.mem.addr);

  return b;
}

#define CTRL_BEGIN        0x0
#define CTRL_SIZE         0x4000
#define TILE_ALLOC_BEGIN  (CTRL_BEGIN + CTRL_SIZE)
#define TILE_ALLOC_SIZE   0x10000
#define TILE_STATE_BEGIN  (TILE_ALLOC_BEGIN + TILE_ALLOC_SIZE)  // 16-byte aligned
#define TILE_STATE_SIZE   0x10000
#define CTX_MEM_TOTAL     (TILE_STATE_BEGIN + TILE_STATE_SIZE)

struct v3d_ctx v3d_ctx_create()
{
  v3d_ctx c;
  c.mem = v3d_mem_create(CTX_MEM_TOTAL, 0x1000,
    MEM_FLAG_COHERENT | MEM_FLAG_ZERO | MEM_FLAG_HINT_PERMALOCK);
  c.offs = 0;
  c.ren_ctrl_start = 0;
  return c;
}

void v3d_ctx_anew(struct v3d_ctx *c, v3d_tex target, uint32_t clear)
{
  c->target = target;
  uint16_t w = target.w, h = target.h;
  const uint16_t bin_sidelen = 32;
  uint8_t bin_cols = (w + bin_sidelen - 1) / bin_sidelen;
  uint8_t bin_rows = (h + bin_sidelen - 1) / bin_sidelen;

  uint8_t *p;
  c->offs = CTRL_BEGIN;

  // Render control

  _align(c->offs, 0x1000);
  p = _armptr(c->mem) + c->offs;
  c->ren_ctrl_start = c->mem.addr + c->offs;

  // Clear Colours
  _putu8(&p, 114);
  _putu32(&p, clear);
  _putu32(&p, clear);
  _putu32(&p, 0);
  _putu8(&p, 0);

  // Tile Rendering Mode Configuration
  _putu8(&p, 113);
  _putu32(&p, target.mem.addr);
  _putu16(&p, w);
  _putu16(&p, h);
  _putu8(&p, (1 << 2) | (1 << 0) | (is_screen(target) ? 0 : (1 << 6)));
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
    _putu32(&p, c->mem.addr + TILE_ALLOC_BEGIN + (y * bin_cols + x) * 32);

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
  _putu32(&p, c->mem.addr + TILE_ALLOC_BEGIN);
  _putu32(&p, TILE_ALLOC_SIZE);
  _putu32(&p, c->mem.addr + TILE_STATE_BEGIN);
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

  c->offs = _offset(p, c->mem);
}

void v3d_ctx_use_batch(struct v3d_ctx *c, const struct v3d_batch *batch)
{
  uint8_t *p = _armptr(c->mem) + c->offs;

  // NV Shader State
  _putu8(&p, 65);
  _putu32(&p, batch->mem.addr);

  c->offs = _offset(p, c->mem);
}

void v3d_ctx_add_call(struct v3d_ctx *c, const struct v3d_call *call)
{
  uint8_t *p = _armptr(c->mem) + c->offs;

  if (call->is_indexed) {
    // Indexed Primitive List
    _putu8(&p, 32);
    _putu8(&p, 20);
    _putu32(&p, call->num_verts);
    _putu32(&p, call->indices.addr);
    _putu32(&p, 0xffffffffu);
  } else {
    // Vertex Array Primitives
    _putu8(&p, 33);
    _putu8(&p, 4);
    _putu32(&p, call->num_verts);
    _putu32(&p, call->start_index);
  }

  c->offs = _offset(p, c->mem);
}

void v3d_ctx_issue(struct v3d_ctx *c)
{
  uint8_t *p = _armptr(c->mem) + c->offs;

  // Flush All State
  _putu8(&p, 5);
  // NOP
  _putu8(&p, 1);
  // Halt
  _putu8(&p, 0);

  c->offs = _offset(p, c->mem);
  c->bin_ctrl_end = c->mem.addr + c->offs;

  // Let's rock

  *V3D_L2CACTL = 4;
  *V3D_SLCACTL = 0x0f0f0f0f;

  *V3D_CT0CS = 0x20;
  while (*V3D_CT0CS & 0x20) co_yield();
  *V3D_BFC = 1;
  *V3D_CT0CA = c->bin_ctrl_start;
  *V3D_CT0EA = c->bin_ctrl_end;
  while (*V3D_BFC == 0) co_yield();

  *V3D_CT1CS = 0x20;
  while (*V3D_CT1CS & 0x20) co_yield();
  *V3D_RFC = 1;
  *V3D_CT1CA = c->ren_ctrl_start;
  *V3D_CT1EA = c->ren_ctrl_end;
}

void v3d_ctx_wait(struct v3d_ctx *c)
{
  if (c->ren_ctrl_start != 0)
    while (*V3D_RFC == 0) co_yield();
  c->ren_ctrl_start = 0;
}
