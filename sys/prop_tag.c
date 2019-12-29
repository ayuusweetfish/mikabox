#include "prop_tag.h"
#include "common.h"
#include "mmu.h"

#define prop_tag(__sz)        \
  volatile struct {           \
    volatile uint32_t size;   \
    volatile uint32_t code;   \
    volatile struct {         \
      volatile uint32_t id;   \
      volatile uint32_t size; \
      volatile uint32_t code; \
      volatile union {        \
        volatile uint32_t u32[(__sz + 3) / 4];  \
        volatile uint16_t u16[(__sz) / 2];      \
        volatile uint8_t u8[__sz];              \
      };                      \
    } tag __attribute__((packed));  \
    volatile uint32_t end_tag;      \
  } __attribute__((packed))

#define prop_tag_init(__buf) do {   \
  (__buf)->size = sizeof *(__buf);  \
  (__buf)->code = 0;                \
  (__buf)->tag.code = 0;            \
  (__buf)->tag.size = sizeof (__buf)->tag.u8; \
  (__buf)->end_tag = 0;             \
} while (0)

#define prop_tag_emit(__buf) do { \
  send_mail(((uint32_t)(__buf) | 0x40000000) >> 4, 8); \
  recv_mail(8); \
} while (0)

// Convenience macros, not for use outside

#define _setup(__sz, __tag) \
  prop_tag(__sz) *buf = mmu_ord_alloc(sizeof(prop_tag(__sz)), 16); \
  prop_tag_init(buf); \
  buf->tag.id = (__tag)

#define _put(__field, __val) (buf->tag.__field = (__val))

#define _get(__field, __var) \
  prop_tag_emit(buf); \
  (__var) = buf->tag.__field; \
  mmu_ord_pop()

#define _emit() \
  prop_tag_emit(buf); \
  mmu_ord_pop()

uint32_t get_clock_rate(uint8_t id)
{
  uint32_t ret;
  _setup(8, 0x30002);
  _put(u32[0], id);
  _get(u32[1], ret);
  return ret;
}

void set_virtual_offs(uint32_t x, uint32_t y)
{
  _setup(8, 0x48009);
  _put(u32[0], x);
  _put(u32[1], y);
  _emit();
}

uint32_t enable_vchiq(uint32_t p)
{
  uint32_t ret;
  _setup(4, 0x48010);
  _put(u32[0], p);
  _get(u32[0], ret);
  return ret;
}
