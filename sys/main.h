#ifndef _Mikabox_main_h_
#define _Mikabox_main_h_

#include <stdint.h>

#define BUF_COUNT 2

extern uint8_t *fb_buf;
extern uint32_t fb_pitch;
extern uint8_t fb_bufid;

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t rgb)
{
  uint8_t *p = fb_buf + y * fb_pitch + x * 3;
  p[0] = (rgb & 0xff);
  p[1] = (rgb >> 8) & 0xff;
  p[2] = (rgb >> 16);
}

#endif
