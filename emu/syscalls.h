#include <stdint.h>

#define FN(__grp, __id)    \
  syscall_##__grp##_##__id

#define init(__fn)

#if !SYSCALLS_DECL && !SYSCALLS_IMPL && !SYSCALLS_TABLE && !SYSCALLS_INIT
#define SYSCALLS_DECL 1
#endif

#if SYSCALLS_DECL
  #define def(__grp, __id, __fn)  \
    uint64_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
#elif SYSCALLS_IMPL
  #define def(__grp, __id, __fn)  \
    uint64_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) \
    { __fn return 0; }
#elif SYSCALLS_TABLE
  #define def(__grp, __id, __fn)  \
    [SYSCALL_GRP_OFFS_##__grp + __id] = &FN(__grp, __id),
#elif SYSCALLS_INIT
  #define def(__grp, __id, __fn)
  #undef init
  #define init(__fn)  __fn
#else
  #define def(__grp, __id, __fn)
#endif

#define SYSCALL_GRP_OFFS_GEN  0
#define SYSCALL_GRP_OFFS_GFX  256
#define SYSCALL_GRP_OFFS_FIL  512
#define SYSCALL_GRP_OFFS_AUD  768
#define SYSCALL_GRP_OFFS_OVW 3840

#if SYSCALLS_DECL
void syscalls_init();
void syscalls_close_app();

#elif SYSCALLS_IMPL
#include "emu.h"
#include "v3d_wrapper.h"
#include "ff_wrapper.h"
#include "audio_wrapper.h"
#include "../ker/pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR DIR_

#define pool_decl_dual(__type, __count, __name) \
  pool_type(__type, __count) __name##_o = { 0 }, __name##_a = { 0 }
#define pool_init_dual(__type, __count, __name) do { \
  pool_init(__type, __count, __name##_o); \
  pool_init(__type, __count, __name##_a); \
} while (0)
#define is_overworld (routine_id >= -1 && routine_id <= 3)
#define dual_sel(__name) (is_overworld ? &__name##_o : &__name##_a)

static pool_decl_dual(v3d_ctx, 16, ctxs);
static pool_decl_dual(v3d_tex, 4096, texs);
static pool_decl_dual(v3d_vertarr, 4096, vas);
static pool_decl_dual(v3d_unifarr, 4096, uas);
static pool_decl_dual(v3d_shader, 256, shaders);
static pool_decl_dual(v3d_batch, 4096, batches);
static pool_decl_dual(v3d_buf, 4096, ias);

static pool_decl_dual(FIL, 4096, files);
static pool_decl_dual(DIR, 256, dirs);

static FILINFO finfo;

#define syscall_log(_fmt, ...) \
  printf("%s: " _fmt, __func__, ##__VA_ARGS__)

void syscall_reinit_rng();
uint64_t syscalls_lcg;
static uint32_t rng_count = 0;

void syscalls_close_app()
{
  #define pool_close(__p, __fn) do { \
    void *__elm; \
    for pool_each(&(__p##_a), __elm) __fn(__elm); \
    pool_clear(&(__p##_a)); \
  } while (0)

  pool_close(ctxs, v3d_ctx_close);
  pool_close(texs, v3d_tex_close);
  pool_close(vas, v3d_vertarr_close);
  pool_close(uas, v3d_unifarr_close);
  pool_close(shaders, v3d_shader_close);
  pool_close(batches, v3d_batch_close);
  pool_close(ias, v3d_idxbuf_close);

  pool_close(files, f_close);
  pool_close(dirs, f_closedir);

  size_t idx;
  pool_alloc(&texs_a, &idx);
}

#endif

init({
  pool_init_dual(v3d_ctx, 16, ctxs);
  pool_init_dual(v3d_tex, 4096, texs);
  pool_init_dual(v3d_vertarr, 4096, vas);
  pool_init_dual(v3d_unifarr, 4096, uas);
  pool_init_dual(v3d_shader, 256, shaders);
  pool_init_dual(v3d_batch, 4096, batches);
  pool_init_dual(v3d_buf, 4096, ias);

  pool_init_dual(FIL, 4096, files);
  pool_init_dual(DIR, 256, dirs);
})

def(GEN, 0, {
  if (routine_id >= 0) {
    syscall_log("Routines can only be changed in initialization routine\n");
    return 0;
  }
  int bank = (routine_id == -1 ? 0 : 4);
  routine_pc[bank + 0] = r0;
  routine_pc[bank + 1] = r1;
  routine_pc[bank + 2] = r2;
  routine_pc[bank + 3] = r3;
})

def(GEN, 1, {
  if (routine_id >= 0 && r0 != 0)
    req_flags &= ~(1 << (routine_id % 4));
})

def(GEN, 2, {
  //update_tick();
  return app_tick;
})

def(GEN, 3, {
  return num_players;
})

def(GEN, 4, {
  if (r0 < num_players)
    return player_btns[r0];
})

def(GEN, 5, {
  if (r0 < num_players)
    return player_axes[r0];
})

def(GEN, 6, {
  if (++rng_count >= 1e6) {
    rng_count = 0;
    syscall_reinit_rng();
  }
  // Newlib/Musl LCG implementation
  uint64_t ret;
  syscalls_lcg = (syscalls_lcg * 6364136223846793005LL + 1);
  ret = (syscalls_lcg >> 32) << 32;
  syscalls_lcg = (syscalls_lcg * 6364136223846793005LL + 1);
  ret = ret | (syscalls_lcg >> 32);
  return ret;
})

def(GEN, 7, {
  char last = '\0';
  char ch;
  for (int i = 0; i < 256; i++) {
    syscall_read_mem(r1++, 1, &ch);
    if (ch == '\0') break;
    putchar(last = ch);
  }
  if (last != '\n') putchar('\n');
})

def(GEN, 128, {
  printf("Syscall with args: %u %u %u %u\n", r0, r1, r2, r3);
})

def(GEN, 129, {
  extern double usec_rate;
  return (uint64_t)(timer_current() * usec_rate);
})

def(GFX, 0, {
  size_t idx;
  v3d_ctx *c = pool_alloc(dual_sel(ctxs), &idx);
  if (c != NULL) {
    *c = v3d_ctx_create();
    return idx;
  } else {
    return (uint32_t)-1;
  }
})

def(GFX, 1, {
  v3d_ctx *c = pool_elm(dual_sel(ctxs), r0);
  v3d_tex *t = pool_elm(dual_sel(texs), r1);
  if (c == NULL || t == NULL) return (uint32_t)-2;
  v3d_ctx_anew(c, *t, r2);
})

def(GFX, 2, {
  v3d_ctx *c = pool_elm(dual_sel(ctxs), r0);
  v3d_batch *b = pool_elm(dual_sel(batches), r1);
  if (c == NULL || b == NULL) return (uint32_t)-2;
  v3d_ctx_use_batch(c, b);
})

def(GFX, 3, {
  v3d_ctx *c = pool_elm(dual_sel(ctxs), r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_call call;
  call.is_indexed = !!r1;
  call.num_verts = r2;
  if (r1) { // Indexed
    v3d_buf *m = pool_elm(dual_sel(ias), r3);
    if (m == NULL) return (uint32_t)-2;
    call.indices = *m;
  } else {
    call.start_index = r3;
  }
  v3d_ctx_add_call(c, &call);
})

def(GFX, 4, {
  v3d_ctx *c = pool_elm(dual_sel(ctxs), r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_issue(c);
})

def(GFX, 5, {
  v3d_ctx *c = pool_elm(dual_sel(ctxs), r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_wait(c);
})

def(GFX, 15, {
  v3d_ctx *c = pool_elm(dual_sel(ctxs), r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_wait(c);
  v3d_ctx_close(c);
  pool_release(dual_sel(ctxs), r0);
})

def(GFX, 16, {
  size_t idx;
  v3d_tex *t = pool_alloc(dual_sel(texs), &idx);
  if (t == NULL) return (uint32_t)-1;
  *t = v3d_tex_create(r0, r1);
  return idx;
})

def(GFX, 17, {
  v3d_tex *t = pool_elm(dual_sel(texs), r0);
  if (t == NULL) return (uint32_t)-2;
  v3d_tex_update(t, r1, (v3d_tex_fmt_t)r2);
})

init({
  size_t idx;
  v3d_tex *t;
  t = pool_alloc(&texs_o, &idx);
  assert(t != NULL && idx == 0);
  t = pool_alloc(&texs_a, &idx);
  assert(t != NULL && idx == 0);
})

def(GFX, 18, {
  v3d_tex *t = pool_elm(dual_sel(texs), 0);
  if (t == NULL) return (uint32_t)-2;
  *t = v3d_tex_screen(r0);
  return 0;
})

def(GFX, 31, {
  if (r0 == 0) return (uint32_t)-3;
  v3d_tex *t = pool_elm(dual_sel(texs), r0);
  if (t == NULL) return (uint32_t)-2;
  v3d_tex_close(t);
  pool_release(dual_sel(texs), r0);
})

def(GFX, 32, {
  size_t idx;
  v3d_vertarr *a = pool_alloc(dual_sel(vas), &idx);
  if (a == NULL) return (uint32_t)-1;
  *a = v3d_vertarr_create(r0, r1);
  return idx;
})

def(GFX, 33, {
  v3d_vertarr *a = pool_elm(dual_sel(vas), r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_vertarr_put(a, r1, r2, r3);
})

def(GFX, 47, {
  v3d_vertarr *a = pool_elm(dual_sel(vas), r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_vertarr_close(a);
  pool_release(dual_sel(vas), r0);
})

def(GFX, 48, {
  size_t idx;
  v3d_unifarr *a = pool_alloc(dual_sel(uas), &idx);
  if (a == NULL) return (uint32_t)-1;
  *a = v3d_unifarr_create(r0);
  return idx;
})

def(GFX, 49, {
  v3d_unifarr *a = pool_elm(dual_sel(uas), r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_unifarr_putu32(a, r1, r2);
})

def(GFX, 50, {
  v3d_unifarr *a = pool_elm(dual_sel(uas), r0);
  v3d_tex *t = pool_elm(dual_sel(texs), r2);
  if (a == NULL || t == NULL) return (uint32_t)-2;
  v3d_unifarr_puttex(a, r1, *t, r3);
})

def(GFX, 63, {
  v3d_unifarr *a = pool_elm(dual_sel(uas), r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_unifarr_close(a);
  pool_release(dual_sel(uas), r0);
})

def(GFX, 64, {
  size_t idx;
  v3d_shader *s = pool_alloc(dual_sel(shaders), &idx);
  if (s == NULL) return (uint32_t)-1;
  *s = v3d_shader_create(r0);
  return idx;
})

def(GFX, 79, {
  v3d_shader *s = pool_elm(dual_sel(shaders), r0);
  if (s == NULL) return (uint32_t)-2;
  v3d_shader_close(s);
  pool_release(dual_sel(shaders), r0);
})

def(GFX, 80, {
  v3d_vertarr *va = pool_elm(dual_sel(vas), r0);
  v3d_unifarr *ua = pool_elm(dual_sel(uas), r1);
  v3d_shader *s = pool_elm(dual_sel(shaders), r2);
  if (va == NULL || ua == NULL || s == NULL)
    return (uint32_t)-2;

  size_t idx;
  v3d_batch *b = pool_alloc(dual_sel(batches), &idx);
  if (b == NULL) return (uint32_t)-1;
  *b = v3d_batch_create(*va, *ua, *s);
  return idx;
})

def(GFX, 95, {
  v3d_batch *b = pool_elm(dual_sel(batches), r0);
  if (b == NULL) return (uint32_t)-2;
  v3d_batch_close(b);
  pool_release(dual_sel(batches), r0);
})

def(GFX, 96, {
  size_t idx;
  v3d_buf *m = pool_alloc(dual_sel(ias), &idx);
  if (m == NULL) return (uint32_t)-1;
  *m = v3d_idxbuf_create(r0);
  return idx;
})

def(GFX, 97, {
  v3d_buf *m = pool_elm(dual_sel(ias), r0);
  if (m == NULL) return (uint32_t)-2;
  v3d_idxbuf_copy(m, r1, r2, r3);
})

def(GFX, 111, {
  v3d_buf *m = pool_elm(dual_sel(ias), r0);
  if (m == NULL) return (uint32_t)-2;
  v3d_idxbuf_close(m);
  pool_release(dual_sel(ias), r0);
})

def(FIL, 0, {
  size_t idx;
  FIL *f = pool_alloc(dual_sel(files), &idx);
  if (f == NULL) return (uint32_t)-1;
  FRESULT r = f_open(f, r0, r1 & 0xff);
  if (r != FR_OK) {
    syscall_log("f_open() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return idx;
})

def(FIL, 1, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_close(f);
  if (r != FR_OK) {
    syscall_log("f_close() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 2, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  UINT br;
  FRESULT r = f_read(f, r1, r2, &br);
  if (r != FR_OK) {
    syscall_log("f_read() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return br;
})

def(FIL, 3, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  UINT bw;
  FRESULT r = f_write(f, r1, r2, &bw);
  if (r != FR_OK) {
    syscall_log("f_write() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return bw;
})

def(FIL, 4, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_lseek(f, (FSIZE_t)r1);
  if (r != FR_OK) {
    syscall_log("f_lseek() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 5, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_truncate(f);
  if (r != FR_OK) {
    syscall_log("f_truncate() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 6, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_sync(f);
  if (r != FR_OK) {
    syscall_log("f_sync() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 7, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  return f_tell(f);
})

def(FIL, 8, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  return f_eof(f);
})

def(FIL, 9, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  return f_size(f);
})

def(FIL, 10, {
  FIL *f = pool_elm(dual_sel(files), r0);
  if (f == NULL) return (uint32_t)-2;
  return f_error(f);
})

def(FIL, 16, {
  size_t idx;
  DIR *d = pool_alloc(dual_sel(dirs), &idx);
  if (d == NULL) return (uint32_t)-1;
  FRESULT r = f_opendir(d, r0);
  if (r != FR_OK) {
    syscall_log("f_opendir() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return idx;
})

def(FIL, 17, {
  DIR *d = pool_elm(dual_sel(dirs), r0);
  if (d == NULL) return (uint32_t)-2;
  FRESULT r = f_closedir(d);
  if (r != FR_OK) {
    syscall_log("f_closedir() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 18, {
  DIR *d = pool_elm(dual_sel(dirs), r0);
  if (d == NULL) return 0;
  FRESULT r = f_readdir(d, &finfo);
  if (r != FR_OK) {
    syscall_log("f_readdir() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
  if (finfo.fname[0] == 0) {
    return 0;
  } else {
    //strcpy((char *)r1, finfo.fname);
    syscall_write_mem(r1, strlen(finfo.fname) + 1, finfo.fname);
    return (finfo.fattrib & AM_DIR) ? 2 : 1;
  }
})

def(FIL, 32, {
  FRESULT r = f_stat(r0, &finfo);
  if (r != FR_OK) {
    if (r != FR_NO_FILE && r != FR_NO_PATH && r != FR_INVALID_NAME)
      syscall_log("f_stat() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
  return (finfo.fattrib & AM_DIR) ? 2 : 1;
})

def(FIL, 33, {
  FRESULT r = f_unlink(r0);
  if (r != FR_OK) {
    syscall_log("f_unlink() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
})

def(FIL, 34, {
  FRESULT r = f_rename(r0, r1);
  if (r != FR_OK) {
    syscall_log("f_rename() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
})

def(FIL, 35, {
  FRESULT r = f_mkdir(r0);
  if (r != FR_OK) {
    syscall_log("f_mkdir() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
})

def(AUD, 0, {
  return audio_blocksize();
})

def(AUD, 1, {
  return audio_dropped();
})

def(AUD, 2, {
  void *p = audio_write_pos();
  syscall_read_mem(r0, audio_blocksize() * 2 * sizeof(int16_t), p);
})

def(OVW, 0, {
  if (request_exec != NULL) free(request_exec);
  request_exec = syscall_dup_str(r0);
})

def(OVW, 1, {
  syscalls_close_app();
})

def(OVW, 2, {
  return (uint32_t)program_paused;
})

def(OVW, 3, {
  request_resume = true;
})

def(OVW, 16, {
})

def(OVW, 17, {
})

#undef def
#undef init
#undef FN

#undef SYSCALLS_DECL
#undef SYSCALLS_IMPL
#undef SYSCALLS_TABLE
#undef SYSCALLS_INIT
