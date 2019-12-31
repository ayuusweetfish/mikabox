#include "v3d.h"
#include "main.h"
#include "common.h"
#include "prop_tag.h"
#include "printf/printf.h"

#include <math.h>

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

#define OFF_VERT    0
#define OFF_TILESTA 0x40000
#define OFF_TILEDAT 0x70000
#define OFF_BIN     0xa0000

#define TEX_W 256
#define TEX_H 128

static const uint32_t aurora[7] = {
  0xbf616a, 0xd08770, 0xebcb8b, 0xa3be8c, 0xb48ead,
  0x88c0d0, 0x5e81ac
};

void v3d_ctx_init(v3d_ctx *ctx, uint32_t w, uint32_t h, void *bufaddr)
{
  ctx->w = w;
  ctx->h = h;
  ctx->bufaddr = (uint32_t)bufaddr;

  uint32_t handle = gpumem_alloc(0x100000, 0x1000, MEM_FLAG_COHERENT | MEM_FLAG_ZERO);
  uint32_t p = gpumem_lock(handle);
  ctx->rhandle = handle;
  ctx->rbusaddr = p;
  ctx->rarmaddr = p & ~GPU_BUS_ADDR;
  v3d_printf("%08x %08x\n", ctx->rbusaddr, ctx->rarmaddr);

  handle = gpumem_alloc(TEX_W * TEX_H * 4 + 0x1000, 0x1000,
    MEM_FLAG_L1_NONALLOCATING | MEM_FLAG_ZERO);
  p = gpumem_lock(handle);
  ctx->thandle = handle;
  ctx->tbusaddr = p;
  ctx->tarmaddr = p & ~GPU_BUS_ADDR;

  uint32_t *q = (uint32_t *)ctx->tarmaddr;
  q[0] = ctx->tbusaddr + 0x1000;
  q[1] = (TEX_W << 8) | (TEX_H << 20);
  q[2] = 0;
  q[3] = 0;

  uint32_t *tex = (uint32_t *)(ctx->tarmaddr + 0x1000);
  for (uint16_t y = 0; y < TEX_H; y++)
  for (uint16_t x = 0; x < TEX_W; x++)
    tex[y * TEX_W + x] = aurora[
      ((x + y * 31 + (x + 2019) % (y + 17)) * (x * 97 + y) + x - y + 19) % 7
    ];

  gpumem_unlock(ctx->thandle);
  gpumem_unlock(ctx->rhandle);
}

static uint32_t qvqshader[] = {
  /* 0x00000000: */ 0x203e303e, 0x100049e0, /* nop; fmul r0, vary, ra15 */
  /* 0x00000008: */ 0x019e7140, 0x10020827, /* fadd r0, r0, r5; nop */
  /* 0x00000010: */ 0x203e303e, 0x100049e1, /* nop; fmul r1, vary, ra15 */
  /* 0x00000018: */ 0x019e7340, 0x10020867, /* fadd r1, r1, r5; nop */
  /* 0x00000020: */ 0x159e7240, 0x10020e67, /* mov t0t, r1; nop */
  /* 0x00000028: */ 0x159e7000, 0x10020e27, /* mov t0s, r0; nop */
  /* 0x00000030: */ 0x009e7000, 0xa00009e7, /* nop; nop; ldtmu0 */
  /* 0x00000038: */ 0x009e7000, 0x400009e7, /* nop; nop; sbwait */
  /* 0x00000040: */ 0x159e7900, 0x30020ba7, /* mov tlbc, r4; nop; thrend */
  /* 0x00000048: */ 0x009e7000, 0x100009e7, /* nop; nop */
  /* 0x00000050: */ 0x009e7000, 0x500009e7, /* nop; nop; sbdone */
};
#define qvqshaderlen (sizeof qvqshader / sizeof qvqshader[0])

void v3d_op(v3d_ctx *ctx)
{
  mem_barrier();
  uint32_t w = ctx->w, h = ctx->h;
  uint8_t *p;
  uint32_t alias = (ctx->rbusaddr & GPU_BUS_ADDR);

  gpumem_lock(ctx->rhandle);

  // Vertices
  p = (uint8_t *)(ctx->rarmaddr) + OFF_VERT;
  uint32_t vertex_start = (uint32_t)p | alias;
  v3d_printf("Vertices start: %p %u\n", p, vertex_start);

  uint32_t t = *TMR_CLO;
  float angle = (float)t / 4e6f * acosf(-1) * 2;

  for (int i = 0; i < 40; i++) {
    for (int j = 0; j < 66; j++) {
      float dx = cos(angle * (1 + i * 0.06f + j * 0.005f)) * 4;
      float dy = sin(angle * (1 + i * 0.06f + j * 0.005f)) * 4;
      _putu16(&p, (uint16_t)((j * 12 + 5 + dx) * 16 + 0.5f));
      _putu16(&p, (uint16_t)((i * 12 + 5 + dy) * 16 + 0.5f));
      _putf32(&p, 1.0f); _putf32(&p, 1.0f);
/*
      int c = aurora[(i + j) * (i - j + i * j + 332) % 7];
      _putf32(&p, (c >> 16) / 255.0f);
      _putf32(&p, ((c >> 8) & 0xff) / 255.0f);
      _putf32(&p, (c & 0xff) / 255.0f);
*/
      _putf32(&p, j / 65.0f);
      _putf32(&p, i / 39.0f);
    }
  }

/*
  _putu16(&p, (uint16_t)(w * (0.2f + 0.07f * cos(angle)) * 16 + 0.5f));
  _putu16(&p, (uint16_t)(h * (0.2f + 0.07f * sin(angle)) * 16 + 0.5f));
  _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  //_putf32(&p, 0.4f); _putf32(&p, 0.7f); _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  _putf32(&p, 0.2f); _putf32(&p, 0.2f);

  _putu16(&p, (uint16_t)(w * 0.2 * 16 + 0.5f));
  _putu16(&p, (uint16_t)(h * 0.8 * 16 + 0.5f));
  _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  //_putf32(&p, 1.0f); _putf32(&p, 1.0f); _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  _putf32(&p, 0.8f); _putf32(&p, 0.2f);

  _putu16(&p, (uint16_t)(w * 0.8 * 16 + 0.5f));
  _putu16(&p, (uint16_t)(h * 0.2 * 16 + 0.5f));
  _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  //_putf32(&p, 1.0f); _putf32(&p, 1.0f); _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  _putf32(&p, 0.2f); _putf32(&p, 0.8f);

  _putu16(&p, (uint16_t)(w * 0.8 * 16 + 0.5f));
  _putu16(&p, (uint16_t)(h * 0.8 * 16 + 0.5f));
  _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  //_putf32(&p, 1.0f); _putf32(&p, 1.0f); _putf32(&p, 1.0f); _putf32(&p, 1.0f);
  _putf32(&p, 0.8f); _putf32(&p, 0.8f);
*/

  v3d_printf("Vertices end: %p\n", p);

  p = (uint8_t *)(((uint32_t)p + 127) & ~127);
  uint32_t vertex_idx_start = (uint32_t)p | alias;
  v3d_printf("Vertex indices start: %p\n", p);

  for (int16_t i = 0; i < 39; i++)
    for (int16_t j = 0; j < 65; j++) {
      _putu16(&p, i * 66 + j);
      _putu16(&p, i * 66 + j + 1);
      _putu16(&p, (i + 1) * 66 + j);
      _putu16(&p, (i + 1) * 66 + (j + 1));
      _putu16(&p, i * 66 + j + 1);
      _putu16(&p, (i + 1) * 66 + j);
    }

/*
  _putu16(&p, 0); _putu16(&p, 1); _putu16(&p, 3);
  _putu16(&p, 0); _putu16(&p, 2); _putu16(&p, 3);
*/
  v3d_printf("Vertex indices end: %p\n", p);

  // Shader
  p = (uint8_t *)(((uint32_t)p + 127) & ~127);
  uint32_t shader_start = (uint32_t)p | alias;
  v3d_printf("Shader instructions start: %p\n", p);

  for (size_t i = 0; i < qvqshaderlen; i++)
    _putu32(&p, qvqshader[i]);
  v3d_printf("Shader instructions end: %p\n", p);

  p = (uint8_t *)(((uint32_t)p + 127) & ~127);
  uint32_t shader_unif_start = (uint32_t)p | alias;
  _putu32(&p, ctx->tbusaddr);
  //_putf32(&p, 0.5f);
  _putu32(&p, 0xff00ff00);
  _putu32(&p, 0xffffff00);
  _putu32(&p, 0xffffffff);

  p = (uint8_t *)(((uint32_t)p + 127) & ~127);
  uint32_t shader_rcd_start = (uint32_t)p | alias;
  v3d_printf("Shader record start: %p\n", p);

  _putu8(&p, 1);
  _putu8(&p, 4 * 4 + 2 * 2);
  _putu8(&p, 0xcc);
  _putu8(&p, 2);    // Number of varyings
  _putu32(&p, shader_start);
  _putu32(&p, ctx->tbusaddr);
  _putu32(&p, vertex_start);
  v3d_printf("Shader record end: %p\n", p);

  // Render control
  p = (uint8_t *)(((uint32_t)p + 127) & ~127);
  uint32_t ren_ctrl_start = (uint32_t)p | alias;
  v3d_printf("Render control start: %p\n", p);

  _putu8(&p, 114);  // GL_CLEAR_COLORS
  _putu32(&p, 0xff2e3440);
  _putu32(&p, 0xff2e3440);
  _putu32(&p, 0);
  _putu8(&p, 0);

  _putu8(&p, 113);  // GL_TILE_RENDER_CONFIG
  _putu32(&p, ctx->bufaddr);
  _putu16(&p, ctx->w);
  _putu16(&p, ctx->h);
  _putu8(&p, 4);
  _putu8(&p, 0);

  _putu8(&p, 115);  // GL_TILE_COORDINATES
  _putu8(&p, 0);
  _putu8(&p, 0);

  _putu8(&p, 28);   // GL_STORE_TILE_BUFFER
  _putu16(&p, 0);
  _putu32(&p, 0);

  uint8_t bin_cols = (w + 63) / 64;
  uint8_t bin_rows = (h + 63) / 64;
  for (uint8_t x = 0; x < bin_cols; x++)
  for (uint8_t y = 0; y < bin_rows; y++) {
    _putu8(&p, 115);  // GL_TILE_COORDINATES
    _putu8(&p, x);
    _putu8(&p, y);

    _putu8(&p, 17);   // GL_BRANCH_TO_SUBLIST
    _putu32(&p, ctx->rbusaddr + OFF_TILEDAT + (y * bin_cols + x) * 32);

    // GL_STORE_MULTISAMPLE(_END)
    _putu8(&p, (x == bin_cols - 1 && y == bin_rows - 1) ? 25 : 24);
  }
  uint32_t ren_ctrl_end = (uint32_t)p | alias;
  v3d_printf("Render control end: %p\n", p);

  // Binning configuration
  p = (uint8_t *)(ctx->rarmaddr) + OFF_BIN;
  uint32_t bin_cfg_start = (uint32_t)p | alias;
  v3d_printf("Binning config start: %p\n", p);

  _putu8(&p, 112);  // GL_TILE_BINNING_CONFIG
  _putu32(&p, ctx->rbusaddr + OFF_TILEDAT);
  _putu32(&p, OFF_BIN - OFF_TILEDAT);
  _putu32(&p, ctx->rbusaddr + OFF_TILESTA);
  _putu8(&p, bin_cols);
  _putu8(&p, bin_rows);
  _putu8(&p, 4);

  _putu8(&p, 6);    // GL_START_TILE_BINNING

  _putu8(&p, 56);   // GL_PRIMITIVE_LIST_FORMAT
  _putu8(&p, 0x32);

  _putu8(&p, 102);  // GL_CLIP_WINDOW
  _putu16(&p, 0);
  _putu16(&p, 0);
  _putu16(&p, w);
  _putu16(&p, h);

  _putu8(&p, 96);   // GL_CONFIG_STATE
  _putu8(&p, 0x3);
  _putu8(&p, 0x0);
  _putu8(&p, 0x2);

  _putu8(&p, 103);  // GL_VIEWPORT_OFFSET
  _putu16(&p, 0);
  _putu16(&p, 0);

  _putu8(&p, 65);   // GL_NV_SHADER_STATE
  _putu32(&p, shader_rcd_start);

  _putu8(&p, 32);   // GL_INDEXED_PRIMITIVE_LIST
  _putu8(&p, 20);   // PRIM_TRIANGLE | 16-bit
  _putu32(&p, 65 * 39 * 6);
  _putu32(&p, vertex_idx_start);
  _putu32(&p, 66 * 40 - 1);

  _putu8(&p, 5);    // GL_FLUSH_ALL_STATE
  _putu8(&p, 1);    // GL_NOP
  _putu8(&p, 0);    // GL_HALT
  uint32_t bin_cfg_end = (uint32_t)p | alias;
  v3d_printf("Binning config end: %p\n", p);

  gpumem_unlock(ctx->rhandle);

  // Let's rock!
  *V3D_L2CACTL = 4;
  *V3D_SLCACTL = 0x0f0f0f0f;

  *V3D_CT0CS = 0x20;
  while (*V3D_CT0CS & 0x20) { }
  *V3D_BFC = 1;
  *V3D_CT0CA = bin_cfg_start;
  *V3D_CT0EA = bin_cfg_end;
  while (*V3D_BFC == 0) { }

  *V3D_CT1CS = 0x20;
  while (*V3D_CT1CS & 0x20) { }
  *V3D_RFC = 1;
  *V3D_CT1CA = ren_ctrl_start;
  *V3D_CT1EA = ren_ctrl_end;
  while (*V3D_RFC == 0) { }
}
