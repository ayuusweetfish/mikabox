#include "common.h"
#include <string.h>

struct framebuffer {
  uint32_t pwidth;
  uint32_t pheight;
  uint32_t vwidth;
  uint32_t vheight;
  uint32_t pitch;
  uint32_t bpp;
  uint32_t xoffs;
  uint32_t yoffs;
  uint32_t buf;
  uint32_t size;
};

static uint8_t *fb_buf;
static uint32_t fb_pitch;

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t rgb)
{
  uint8_t *p = fb_buf + y * fb_pitch + x * 3;
  p[0] = (rgb & 0xff);
  p[1] = (rgb >> 8) & 0xff;
  p[2] = (rgb >> 16);
}

void sys_main()
{
  volatile struct framebuffer f __attribute__((aligned(16))) = { 0 };
  f.pwidth = 800; f.pheight = 480;
  f.vwidth = 800; f.vheight = 480;
  f.bpp = 24;
  // 1: channel for framebuffer
  send_mail(((uint32_t)&f + 0xc0000000) >> 4, 1);
  recv_mail(1);

  fb_buf = (uint8_t *)(f.buf);
  fb_pitch = f.pitch;

  for (uint32_t y = 0; y < 480; y++)
    for (uint32_t x = 0; x < 800; x++)
      put_pixel(x, y, 0x2e3440);

  mem_barrier();
  *GPFSEL4 |= (1 << 21);
  while (1) {
    mem_barrier();
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
  }
}
