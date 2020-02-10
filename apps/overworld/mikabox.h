#ifndef _Mikabox_mikabox_h_
#define _Mikabox_mikabox_h_

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

// Constants

#define BTN_U     (1 << 0)
#define BTN_D     (1 << 1)
#define BTN_L     (1 << 2)
#define BTN_R     (1 << 3)
#define BTN_A     (1 << 4)
#define BTN_B     (1 << 5)
#define BTN_X     (1 << 6)
#define BTN_Y     (1 << 7)
#define BTN_L1    (1 << 8)
#define BTN_R1    (1 << 9)
#define BTN_L2    (1 << 10)
#define BTN_R2    (1 << 11)
#define BTN_L3    (1 << 12)
#define BTN_R3    (1 << 13)
#define BTN_START (1 << 14)
#define BTN_OPTN  (1 << 15)
#define BTN_META  (1 << 16)
#define BTN_AUX   (1 << 17)

#define BTN_CRO   BTN_A
#define BTN_CIR   BTN_B
#define BTN_SQR   BTN_X
#define BTN_TRI   BTN_Y

enum {
  v3d_tex_fmt_rgb = 0,
  v3d_tex_fmt_bgr,
  v3d_tex_fmt_rgba,
  v3d_tex_fmt_bgra,
  v3d_tex_fmt_argb,
  v3d_tex_fmt_abgr,
};

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

#define FA_READ           0x01
#define FA_WRITE          0x02
#define FA_OPEN_EXISTING  0x00
#define FA_CREATE_NEW     0x04
#define FA_CREATE_ALWAYS  0x08
#define FA_OPEN_ALWAYS    0x10
#define FA_OPEN_APPEND    0x30

// System call fundamentals

uint64_t syscall0(uint32_t);
uint64_t syscall1(uint32_t, uint32_t);
uint64_t syscall2(uint32_t, uint32_t, uint32_t);
uint64_t syscall3(uint32_t, uint32_t, uint32_t, uint32_t);
uint64_t syscall4(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

// System call definitions

static inline void mika_rout(const void *c1, const void *c2, const void *c3, const void *c4) { syscall4((uint32_t)c1, (uint32_t)c2, (uint32_t)c3, (uint32_t)c4, 0); }
static inline void mika_yield(uint32_t clear) { syscall1(clear, 1); }
static inline uint64_t mika_tick() { return syscall0(2); }
static inline uint32_t mika_party() { return syscall0(3); }
static inline uint64_t mika_btns(uint32_t player) { return syscall1(player, 4); }
static inline uint64_t mika_axes(uint32_t player) { return syscall1(player, 5); }
static inline uint64_t mika_rand() { return syscall0(6); }
static inline void mika_log(uint32_t level, const void *str) { syscall2(level, (uint32_t)str, 7); }
static inline void mika_test() { syscall0(128); }
static inline uint64_t mika_wall() { return syscall0(129); }
static inline uint32_t gfx_ctx_create() { return syscall0(256); }
static inline void gfx_ctx_reset(uint32_t ctx, uint32_t target, uint32_t clear) { syscall3(ctx, target, clear, 257); }
static inline void gfx_ctx_batch(uint32_t ctx, uint32_t bat) { syscall2(ctx, bat, 258); }
static inline void gfx_ctx_call(uint32_t ctx, uint32_t is_indexed, uint32_t num_verts, uint32_t start_or_idxbuf) { syscall4(ctx, is_indexed, num_verts, start_or_idxbuf, 259); }
static inline void gfx_ctx_issue(uint32_t ctx) { syscall1(ctx, 260); }
static inline void gfx_ctx_wait(uint32_t ctx) { syscall1(ctx, 261); }
static inline void gfx_ctx_close(uint32_t ctx) { syscall1(ctx, 271); }
static inline uint32_t gfx_tex_create(uint32_t width, uint32_t height) { return syscall2(width, height, 272); }
static inline void gfx_tex_update(uint32_t tex, const void *pixels, uint32_t format) { syscall3(tex, (uint32_t)pixels, format, 273); }
static inline uint32_t gfx_tex_screen() { return syscall0(274); }
static inline void gfx_tex_close(uint32_t tex) { syscall1(tex, 287); }
static inline uint32_t gfx_varr_create(uint32_t num_verts, uint32_t num_varyings) { return syscall2(num_verts, num_varyings, 288); }
static inline void gfx_varr_put(uint32_t varr, uint32_t start, const void *verts, uint32_t num) { syscall4(varr, start, (uint32_t)verts, num, 289); }
static inline void gfx_varr_close(uint32_t varr) { syscall1(varr, 303); }
static inline uint32_t gfx_uarr_create(uint32_t num_uniforms) { return syscall1(num_uniforms, 304); }
static inline void gfx_uarr_putu32(uint32_t uarr, uint32_t index, uint32_t value) { syscall3(uarr, index, value, 305); }
static inline void gfx_uarr_puttex(uint32_t uarr, uint32_t index, uint32_t tex, uint32_t config) { syscall4(uarr, index, tex, config, 306); }
static inline void gfx_uarr_close(uint32_t uarr) { syscall1(uarr, 319); }
static inline uint32_t gfx_shad_create(const void *code) { return syscall1((uint32_t)code, 320); }
static inline void gfx_shad_close(uint32_t shad) { syscall1(shad, 335); }
static inline uint32_t gfx_bat_create(uint32_t varr, uint32_t uarr, uint32_t shad) { return syscall3(varr, uarr, shad, 336); }
static inline void gfx_bat_close(uint32_t bat) { syscall1(bat, 351); }
static inline uint32_t gfx_iarr_create(uint32_t num) { return syscall1(num, 352); }
static inline uint32_t gfx_iarr_put(uint32_t iarr, uint32_t start, const void *idxs, uint32_t num) { return syscall4(iarr, start, (uint32_t)idxs, num, 353); }
static inline uint32_t gfx_iarr_close(uint32_t iarr) { return syscall1(iarr, 367); }
static inline uint32_t fil_open(const void *path, uint32_t flags) { return syscall2((uint32_t)path, flags, 512); }
static inline void fil_close(uint32_t f) { syscall1(f, 513); }
static inline uint32_t fil_read(uint32_t f, void *buf, uint32_t len) { return syscall3(f, (uint32_t)buf, len, 514); }
static inline uint32_t fil_write(uint32_t f, const void *buf, uint32_t len) { return syscall3(f, (uint32_t)buf, len, 515); }
static inline void fil_seek(uint32_t f, uint32_t pos) { syscall2(f, pos, 516); }
static inline void fil_trunc(uint32_t f) { syscall1(f, 517); }
static inline void fil_flush(uint32_t f) { syscall1(f, 518); }
static inline uint32_t fil_tell(uint32_t f) { return syscall1(f, 519); }
static inline uint32_t fil_eof(uint32_t f) { return syscall1(f, 520); }
static inline uint32_t fil_size(uint32_t f) { return syscall1(f, 521); }
static inline uint32_t fil_err(uint32_t f) { return syscall1(f, 522); }
static inline uint32_t fil_opendir(const void *path) { return syscall1((uint32_t)path, 528); }
static inline void fil_closedir(uint32_t d) { syscall1(d, 529); }
static inline uint32_t fil_readdir(uint32_t d, void *name) { return syscall2(d, (uint32_t)name, 530); }
static inline uint32_t fil_stat(const void *path) { return syscall1((uint32_t)path, 544); }
static inline void fil_unlink(const void *path) { syscall1((uint32_t)path, 545); }
static inline void fil_rename(const void *path_old, const void *path_new) { syscall2((uint32_t)path_old, (uint32_t)path_new, 546); }
static inline void fil_mkdir(const void *path) { syscall1((uint32_t)path, 547); }
static inline uint32_t aud_blocksize() { return syscall0(768); }
static inline uint32_t aud_dropped() { return syscall0(769); }
static inline void aud_write(void *buf) { syscall1((uint32_t)buf, 770); }
static inline uint32_t ovw_start(const void *path) { return syscall1((uint32_t)path, 3840); }
static inline void ovw_stop() { syscall0(3841); }
static inline uint32_t ovw_paused() { return syscall0(3842); }
static inline void ovw_resume() { syscall0(3843); }

// Utilities

static inline void mika_printf(const char *format, ...)
{
  char s[256];
  va_list arg;
  va_start(arg, format);
  vsnprintf(s, sizeof s, format, arg);
  va_end(arg);
  mika_log(0, s);
}

#define printf mika_printf

#endif
