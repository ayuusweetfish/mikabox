#include "prop_tag.h"
#include "regs.h"
#include "mmu.h"

void send_mail(uint32_t data, uint8_t channel)
{
  mem_barrier();
  while ((*MAIL0_STATUS) & (1u << 31)) { }
  *MAIL0_WRITE = (data << 4) | (channel & 15);
  mem_barrier();
}

uint32_t recv_mail(uint8_t channel)
{
  mem_barrier();
  do {
    while ((*MAIL0_STATUS) & (1u << 30)) { }
    uint32_t data = *MAIL0_READ;
    if ((data & 15) == channel) {
      mem_barrier();
      return (data >> 4);
    }
  } while (1);
}

#include <string.h>

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
  uint32_t ret; \
  prop_tag(__sz) *buf = mmu_ord_alloc(sizeof(prop_tag(__sz)), 16); \
  prop_tag_init(buf); \
  buf->tag.id = (__tag)

#define _put(__field, __val) (buf->tag.__field = (__val))

#define _get(__field) \
  prop_tag_emit(buf); \
  ret = buf->tag.__field; \
  mmu_ord_pop()

#define _copy(__size, __dest) \
  prop_tag_emit(buf); \
  memcpy((__dest), (void *)buf->tag.u8, (__size)); \
  (void)ret; \
  mmu_ord_pop()

#define _emit() \
  prop_tag_emit(buf); \
  (void)ret; \
  mmu_ord_pop()

uint32_t get_clock_rate(uint8_t id)
{
  _setup(8, 0x30002);
  _put(u32[0], id);
  _get(u32[1]);
  return ret;
}

void set_clock_rate(uint8_t id, uint32_t hz)
{
  _setup(8, 0x38002);
  _put(u32[0], id);
  _put(u32[1], hz);
  _emit();
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
  _setup(4, 0x48010);
  _put(u32[0], p);
  _get(u32[0]);
  return ret;
}

void enable_qpu()
{
  _setup(4, 0x30012);
  _put(u32[0], 1);
  _emit();
}

bool set_power_state(uint32_t device, uint32_t state)
{
  _setup(8, 0x28001);
  _put(u32[0], device);
  _put(u32[1], state);
  _get(u32[0]);
  return (ret & 1);
}

void get_mac_addr(uint8_t *addr)
{
  _setup(6, 0x10003);
  _copy(6, addr);
}

uint64_t get_arm_memory()
{
  _setup(8, 0x10005);
  uint64_t r = 0;
  _get(u32[0]); r = (uint64_t)ret << 32;
  _get(u32[1]); r |= ret;
  return r;
}

uint64_t get_gpu_memory()
{
  _setup(8, 0x10006);
  uint64_t r = 0;
  _get(u32[0]); r = (uint64_t)ret << 32;
  _get(u32[1]); r |= ret;
  return r;
}

uint32_t gpumem_alloc(uint32_t size, uint32_t align, uint32_t flags)
{
  _setup(12, 0x3000c);
  _put(u32[0], size);
  _put(u32[1], align);
  _put(u32[2], flags);
  _get(u32[0]);
  return ret;
}

uint32_t gpumem_lock(uint32_t handle)
{
  _setup(4, 0x3000d);
  _put(u32[0], handle);
  _get(u32[0]);
  return ret;
}

void gpumem_unlock(uint32_t handle)
{
  _setup(4, 0x3000e);
  _put(u32[0], handle);
  _emit();
}

void gpumem_release(uint32_t handle)
{
  _setup(4, 0x3000f);
  _put(u32[0], handle);
  _emit();
}
