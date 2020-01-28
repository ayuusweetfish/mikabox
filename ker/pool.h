#ifndef _Mikabox_pool_h_
#define _Mikabox_pool_h_

#include <stddef.h>
#include <stdint.h>

#define pool_type(__type, __count) struct { \
  size_t sz, cnt; \
  size_t elm_offs, used_offs; \
  __type elm[__count]; \
  uint32_t used[(__count + 31) / 32]; \
}

#define pool_decl(__type, __count, __name) \
  pool_type(__type, __count) __name = { 0 };

#define pool_init(__type, __count, __name) do { \
  __name.sz = sizeof(__type); \
  __name.cnt = (__count); \
  __name.elm_offs = (uint8_t *)&__name.elm - (uint8_t *)&__name; \
  __name.used_offs = (uint8_t *)&__name.used - (uint8_t *)&__name; \
} while (0)

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

#endif
