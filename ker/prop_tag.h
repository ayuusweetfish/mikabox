#ifndef _Mikabox_prop_tag_h_
#define _Mikabox_prop_tag_h_

#include <stdbool.h>
#include <stdint.h>

uint32_t get_clock_rate(uint8_t id);
void set_clock_rate(uint8_t id, uint32_t hz);
void set_virtual_offs(uint32_t x, uint32_t y);
uint32_t enable_vchiq(uint32_t p);
void enable_qpu();
bool set_power_state(uint32_t device, uint32_t state);
void get_mac_addr(uint8_t *addr);

// Taken from https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
enum {
  MEM_FLAG_DISCARDABLE = 1 << 0, /* can be resized to 0 at any time. Use for cached data */
  MEM_FLAG_NORMAL = 0 << 2, /* normal allocating alias. Don't use from ARM */
  MEM_FLAG_DIRECT = 1 << 2, /* 0xC alias uncached */
  MEM_FLAG_COHERENT = 2 << 2, /* 0x8 alias. Non-allocating in L2 but coherent */
  MEM_FLAG_L1_NONALLOCATING = (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT), /* Allocating in L2 */
  MEM_FLAG_ZERO = 1 << 4,  /* initialise buffer to all zeros */
  MEM_FLAG_NO_INIT = 1 << 5, /* don't initialise (default is initialise to all ones */
  MEM_FLAG_HINT_PERMALOCK = 1 << 6, /* Likely to be locked for long periods of time. */
};
// Returns handle
uint32_t gpumem_alloc(uint32_t size, uint32_t align, uint32_t flags);
// Returns bus address
uint32_t gpumem_lock(uint32_t handle);
void gpumem_unlock(uint32_t handle);
void gpumem_release(uint32_t handle);

#endif
