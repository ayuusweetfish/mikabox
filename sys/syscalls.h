#include <stdint.h>

#define FN(__grp, __id)    \
  syscall_##__grp##_##__id

#define init(__fn)

#if SYSCALLS_DECL
  #define def(__grp, __id, __fn)  \
    uint32_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
#elif SYSCALLS_IMPL
  #define def(__grp, __id, __fn)  \
    uint32_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) \
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

#if SYSCALLS_DECL
void syscalls_init();

#elif SYSCALLS_IMPL
#include "printf/printf.h"
#include "common.h"
#include "v3d.h"
#include "fatfs/ff.h"
#include <string.h>

#define pool_type(__type, __count) struct { \
  const size_t sz, cnt; \
  const size_t elm_offs, used_offs; \
  __type elm[__count]; \
  uint32_t used[(__count + 31) / 32]; \
}

#define pool_decl(__type, __count, __name) \
  pool_type(__type, __count) __name = { \
    .sz = sizeof(__type), \
    .cnt = (__count), \
    .elm_offs = (uint8_t *)&__name.elm - (uint8_t *)&__name, \
    .used_offs = (uint8_t *)&__name.used - (uint8_t *)&__name, \
    .used = { 0 }, \
  }

#define pool_sz(__p)  (*((size_t *)(__p) + 0))
#define pool_cnt(__p) (*((size_t *)(__p) + 1))
#define pool_elm_offs(__p)  (*((size_t *)(__p) + 2))
#define pool_used_offs(__p) (*((size_t *)(__p) + 3))

#define pool_elm_direct(__p, __i) \
  ((void *)((uint8_t *)(__p) + pool_elm_offs(__p) + (__i) * pool_sz(__p)))
#define pool_used(__p) \
  ((uint32_t *)((uint8_t *)(__p) + pool_used_offs(__p)))

static inline void *pool_alloc(void *p, size_t *idx)
{
  size_t cnt = pool_cnt(p);
  uint32_t *used = pool_used(p);
  for (size_t i = 0, j = 0; i < cnt; i += 32, j += 1) {
    if (used[j] != 0xffffffff) {
      size_t bit = __builtin_ctz(~used[j]);
      if (i + bit < cnt) {
        used[j] |= (1 << bit);
        *idx = i + bit;
        return pool_elm_direct(p, i + bit);
      }
    }
  }
  return NULL;
}

static inline void pool_release(void *p, size_t idx)
{
  size_t cnt = pool_cnt(p);
  uint32_t *used = pool_used(p);
  if (idx < cnt)
    used[idx / 32] &= ~(1 << (idx % 32));
}

static inline void *pool_elm(void *p, size_t idx)
{
  uint32_t *used = pool_used(p);
  return (idx < pool_cnt(p) && (used[idx / 32] & (1 << (idx % 32)))) ?
    pool_elm_direct(p, idx) : NULL;
}

static pool_decl(v3d_ctx, 16, ctxs);
static pool_decl(v3d_tex, 4096, texs);
static pool_decl(v3d_vertarr, 4096, vas);
static pool_decl(v3d_unifarr, 4096, uas);
static pool_decl(v3d_shader, 256, shaders);
static pool_decl(v3d_batch, 4096, batches);
static pool_decl(v3d_mem, 4096, ias);

static pool_decl(FIL, 4096, files);
static pool_decl(DIR, 256, dirs);
static FILINFO finfo;

#define syscall_log(_fmt, ...) \
  printf("%s: " _fmt, __func__, ##__VA_ARGS__)

static inline const char *f_strerr(FRESULT fr)
{
  static const char *const strs[] = {
    "FR_OK",
    "FR_DISK_ERR",
    "FR_INT_ERR",
    "FR_NOT_READY",
    "FR_NO_FILE",
    "FR_NO_PATH",
    "FR_INVALID_NAME",
    "FR_DENIED",
    "FR_EXIST",
    "FR_INVALID_OBJECT",
    "FR_WRITE_PROTECTED",
    "FR_INVALID_DRIVE",
    "FR_NOT_ENABLED",
    "FR_NO_FILESYSTEM",
    "FR_MKFS_ABORTED",
    "FR_TIMEOUT",
    "FR_LOCKED",
    "FR_NOT_ENOUGH_CORE",
    "FR_TOO_MANY_OPEN_FILES",
    "FR_INVALID_PARAMETER",
  };
  return strs[fr];
}
#endif

def(GEN, 6, {
  mem_barrier();
  uint32_t data = *RNG_DATA;
  mem_barrier();
  return data;
})

def(GEN, 43, {
  printf("from syscall! %u\n", r0);
})

def(GFX, 0, {
  size_t idx;
  v3d_ctx *c = pool_alloc(&ctxs, &idx);
  if (c != NULL) {
    *c = v3d_ctx_create();
    return idx;
  } else {
    return (uint32_t)-1;
  }
})

def(GFX, 1, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  v3d_tex *t = pool_elm(&texs, r1);
  if (c == NULL || t == NULL) return (uint32_t)-2;
  v3d_ctx_anew(c, *t, r2);
})

def(GFX, 2, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  v3d_batch *b = pool_elm(&batches, r1);
  if (c == NULL || b == NULL) return (uint32_t)-2;
  v3d_ctx_use_batch(c, b);
})

def(GFX, 3, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_call call;
  call.is_indexed = !!r1;
  call.num_verts = r2;
  if (r1) { // Indexed
    v3d_mem *m = pool_elm(&ias, r3);
    if (m == NULL) return (uint32_t)-2;
    call.indices = *m;
  } else {
    call.start_index = r3;
  }
  v3d_ctx_add_call(c, &call);
})

def(GFX, 4, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_issue(c);
})

def(GFX, 5, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_wait(c);
})

def(GFX, 15, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_wait(c);
  v3d_close(c);
  pool_release(&ctxs, r0);
})

def(GFX, 16, {
  size_t idx;
  v3d_tex *t = pool_alloc(&texs, &idx);
  if (t == NULL) return (uint32_t)-1;
  *t = v3d_tex_create(r0, r1);
  return idx;
})

def(GFX, 17, {
  v3d_tex *t = pool_elm(&texs, r0);
  if (t == NULL) return (uint32_t)-2;
  v3d_tex_update(t, (uint8_t *)r1, (v3d_tex_fmt_t)r2);
})

init({
  size_t idx;
  v3d_tex *t = pool_alloc(&texs, &idx);
  assert(t != NULL && idx == 0);
})

def(GFX, 18, {
  v3d_tex *t = pool_elm(&texs, 0);
  if (t == NULL) return (uint32_t)-2;
  *t = v3d_tex_screen(r0);
  return 0;
})

def(GFX, 31, {
  if (r0 == 0) return (uint32_t)-3;
  v3d_tex *t = pool_elm(&texs, r0);
  if (t == NULL) return (uint32_t)-2;
  v3d_close(t);
  pool_release(&texs, r0);
})

def(GFX, 32, {
  size_t idx;
  v3d_vertarr *a = pool_alloc(&vas, &idx);
  if (a == NULL) return (uint32_t)-1;
  *a = v3d_vertarr_create(r0, r1);
  return idx;
})

def(GFX, 33, {
  v3d_vertarr *a = pool_elm(&vas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_vertarr_put(a, r1, (const v3d_vert *)r2, r3);
})

def(GFX, 47, {
  v3d_vertarr *a = pool_elm(&vas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_close(a);
  pool_release(&vas, r0);
})

def(GFX, 48, {
  size_t idx;
  v3d_unifarr *a = pool_alloc(&uas, &idx);
  if (a == NULL) return (uint32_t)-1;
  *a = v3d_unifarr_create(r0);
  return idx;
})

def(GFX, 49, {
  v3d_unifarr *a = pool_elm(&uas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_unifarr_putu32(a, r1, r2);
})

def(GFX, 50, {
  v3d_unifarr *a = pool_elm(&uas, r0);
  v3d_tex *t = pool_elm(&texs, r2);
  if (a == NULL || t == NULL) return (uint32_t)-2;
  v3d_unifarr_puttex(a, r1, *t, r3);
})

def(GFX, 63, {
  v3d_unifarr *a = pool_elm(&uas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_close(a);
  pool_release(&uas, r0);
})

def(GFX, 64, {
  size_t idx;
  v3d_shader *s = pool_alloc(&shaders, &idx);
  if (s == NULL) return (uint32_t)-1;
  *s = v3d_shader_create((const char *)r0);
  return idx;
})

def(GFX, 79, {
  v3d_shader *s = pool_elm(&shaders, r0);
  if (s == NULL) return (uint32_t)-2;
  v3d_close(s);
  pool_release(&shaders, r0);
})

def(GFX, 80, {
  v3d_vertarr *va = pool_elm(&vas, r0);
  v3d_unifarr *ua = pool_elm(&uas, r1);
  v3d_shader *s = pool_elm(&shaders, r2);
  if (va == NULL || ua == NULL || s == NULL)
    return (uint32_t)-2;

  size_t idx;
  v3d_batch *b = pool_alloc(&batches, &idx);
  if (b == NULL) return (uint32_t)-1;
  *b = v3d_batch_create(*va, *ua, *s);
  return idx;
})

def(GFX, 95, {
  v3d_batch *b = pool_elm(&batches, r0);
  if (b == NULL) return (uint32_t)-2;
  v3d_close(b);
  pool_release(&batches, r0);
})

def(GFX, 96, {
  size_t idx;
  v3d_mem *m = pool_alloc(&ias, &idx);
  if (m == NULL) return (uint32_t)-1;
  *m = v3d_mem_indexbuf(r0);
  return idx;
})

def(GFX, 97, {
  v3d_mem *m = pool_elm(&ias, r0);
  if (m == NULL) return (uint32_t)-2;
  v3d_mem_indexcopy(m, r1, (void *)r2, r3);
})

def(GFX, 111, {
  v3d_mem *m = pool_elm(&ias, r0);
  if (m == NULL) return (uint32_t)-2;
  v3d_mem_close(m);
  pool_release(&ias, r0);
})

def(FIL, 0, {
  size_t idx;
  FIL *f = pool_alloc(&files, &idx);
  if (f == NULL) return (uint32_t)-1;
  FRESULT r = f_open(f, (const char *)r0, r1 & 0xff);
  if (r != FR_OK) {
    syscall_log("f_open() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return idx;
})

def(FIL, 1, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_close(f);
  if (r != FR_OK) {
    syscall_log("f_close() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 2, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  UINT br;
  FRESULT r = f_read(f, (void *)r1, r2, &br);
  if (r != FR_OK) {
    syscall_log("f_read() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return br;
})

def(FIL, 3, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  UINT bw;
  FRESULT r = f_write(f, (void *)r1, r2, &bw);
  if (r != FR_OK) {
    syscall_log("f_write() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return bw;
})

def(FIL, 4, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_lseek(f, (FSIZE_t)r1);
  if (r != FR_OK) {
    syscall_log("f_lseek() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 5, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_truncate(f);
  if (r != FR_OK) {
    syscall_log("f_truncate() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 6, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  FRESULT r = f_sync(f);
  if (r != FR_OK) {
    syscall_log("f_sync() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 7, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  return f_tell(f);
})

def(FIL, 8, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  return f_eof(f);
})

def(FIL, 9, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  return f_size(f);
})

def(FIL, 10, {
  FIL *f = pool_elm(&files, r0);
  if (f == NULL) return (uint32_t)-2;
  return f_error(f);
})

def(FIL, 16, {
  size_t idx;
  DIR *d = pool_alloc(&dirs, &idx);
  if (d == NULL) return (uint32_t)-1;
  FRESULT r = f_opendir(d, (const char *)r0);
  if (r != FR_OK) {
    syscall_log("f_opendir() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
  return idx;
})

def(FIL, 17, {
  DIR *d = pool_elm(&dirs, r0);
  if (d == NULL) return (uint32_t)-2;
  FRESULT r = f_closedir(d);
  if (r != FR_OK) {
    syscall_log("f_closedir() returns %d (%s)\n", (int)r, f_strerr(r));
    return (uint32_t)-3;
  }
})

def(FIL, 18, {
  DIR *d = pool_elm(&dirs, r0);
  if (d == NULL) return 0;
  FRESULT r = f_readdir(d, &finfo);
  if (r != FR_OK) {
    syscall_log("f_readdir() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
  if (finfo.fname[0] == 0) {
    return 0;
  } else {
    char *fname = (char *)r1;
    strcpy(fname, finfo.fname);
    return (finfo.fattrib & AM_DIR) ? 2 : 1;
  }
})

def(FIL, 32, {
  FRESULT r = f_stat((const char *)r0, &finfo);
  if (r != FR_OK) {
    if (r != FR_NO_FILE && r != FR_NO_PATH && r != FR_INVALID_NAME)
      syscall_log("f_stat() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
  return (finfo.fattrib & AM_DIR) ? 2 : 1;
})

def(FIL, 33, {
  FRESULT r = f_unlink((const char *)r0);
  if (r != FR_OK) {
    syscall_log("f_unlink() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
})

def(FIL, 34, {
  FRESULT r = f_rename((const char *)r0, (const char *)r1);
  if (r != FR_OK) {
    syscall_log("f_rename() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
})

def(FIL, 35, {
  FRESULT r = f_mkdir((const char *)r0);
  if (r != FR_OK) {
    syscall_log("f_mkdir() returns %d (%s)\n", (int)r, f_strerr(r));
    return 0;
  }
})

#undef def
#undef init
#undef FN
