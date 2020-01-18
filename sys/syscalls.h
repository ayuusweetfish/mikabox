#include <stdint.h>

#define FN(__grp, __id)    \
  syscall_##__grp##_##__id

#if SYSCALLS_DECL
  #define def(__grp, __id, __fn)   \
    void FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
#elif SYSCALLS_IMPL
  #define def(__grp, __id, __fn)   \
    void FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) __fn
#elif SYSCALLS_TABLE
  #define def(__grp, __id, __fn)   \
    [SYSCALL_GRP_OFFS_##__grp + __id] = &FN(__grp, __id),
#else
  #define def(__grp, __id, __fn)
#endif

#define SYSCALL_GRP_OFFS_GEN  0
#define SYSCALL_GRP_OFFS_GFX  256

#if SYSCALLS_IMPL
#include "printf/printf.h"
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

#define pool_used(__p) \
  ((uint32_t *)((uint8_t *)(__p) + pool_used_offs(__p)))

static inline size_t pool_get(void *p)
{
  size_t cnt = pool_cnt(p);
  uint32_t *used = pool_used(p);
  for (size_t i = 0, j = 0; i < cnt; i += 32, j += 1) {
    if (used[j] != 0xffffffff) {
      size_t bit = __builtin_ctz(~used[j]);
      if (i + bit < cnt) {
        used[j] |= (1 << bit);
        return i + bit;
      }
    }
  }
  return cnt;
}

static inline void pool_release(void *p, size_t idx)
{
  size_t cnt = pool_cnt(p);
  uint32_t *used = pool_used(p);
  if (idx < cnt)
    used[idx / 32] &= ~(1 << (idx % 32));
}

static pool_decl(v3d_ctx, 16, ctxs);
static pool_decl(v3d_tex, 4096, texs);
static pool_decl(v3d_vertarr, 4096, vas);
static pool_decl(v3d_unifarr, 4096, uas);
static pool_decl(v3d_shader, 256, shaders);
static pool_decl(v3d_batch, 4096, batches);
#endif

def(GEN, 43, {
  printf("from syscall! %u\n", r0);
})

def(GFX, 0, {
  printf("graphics!\n");
  for (uint32_t i = 0; i < 40; i++)
    printf(" %zu", pool_get(&shaders));
  printf("\n");
  for (uint32_t i = 1; i < 40; i += 2) pool_release(&shaders, i);
  for (uint32_t i = 0; i < 40; i++)
    printf(" %zu", pool_get(&shaders));
  printf("\n");
})

#undef impl
#undef def
#undef FN
