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

#if SYSCALLS_DECL
void syscalls_init();

#elif SYSCALLS_IMPL
#include "printf/printf.h"
#include "common.h"
#include "v3d.h"

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
v3d_ctx *zz;
v3d_tex *yy;  // LOOK AT ME YOU NEED TO SET THIS
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
    zz = c;
    return idx;
  } else {
    return (uint32_t)-1;
  }
})

def(GFX, 1, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  v3d_tex *t = pool_elm(&texs, r1);
  //if (c == NULL || t == NULL) return 0;
  //v3d_ctx_anew(c, *t, r2);
  printf("gfx 1: %u %u\n", r0, r1);
  v3d_ctx_anew(zz, *yy, 0xffadbecf);
})

def(GFX, 2, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  v3d_batch *b = pool_elm(&batches, r1);
  if (c == NULL || b == NULL) return 0;
  v3d_ctx_use_batch(c, b);
})

def(GFX, 3, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return 0;
  v3d_call call;
  call.is_indexed = !!r0;
  call.num_verts = r1;
  if (r0) { // Indexed
    v3d_mem *m = pool_elm(&ias, r2);
    if (m == NULL) return 0;
    call.indices = *m;
  } else {
    call.start_index = r2;
  }
  v3d_ctx_add_call(c, &call);
})

def(GFX, 4, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return 0;
  v3d_ctx_issue(c);
})

def(GFX, 5, {
  /*
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return 0;
  v3d_ctx_wait(c);
  */
  v3d_ctx_wait(zz);
})

init({
  size_t idx;
  v3d_tex *t = pool_alloc(&texs, &idx);
  assert(t != NULL && idx == 0);
})

def(GFX, 18, {
  v3d_tex *t = pool_elm(&texs, 0);
  if (t == NULL) return (uint32_t)-1;
  *t = v3d_tex_screen(r0);
  yy = t;
  return 0;
})

#undef def
#undef init
#undef FN
