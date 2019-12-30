#ifndef _Mikabox_prop_tag_h_
#define _Mikabox_prop_tag_h_

#include <stdbool.h>
#include <stdint.h>

uint32_t get_clock_rate(uint8_t id);
void set_clock_rate(uint8_t id, uint32_t hz);
void set_virtual_offs(uint32_t x, uint32_t y);
uint32_t enable_vchiq(uint32_t p);
void enable_qpu();

#endif
