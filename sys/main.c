#include "main.h"
#include "common.h"
#include "charbuf.h"

#include "printf/printf.h"

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

uint8_t *fb_buf;
uint32_t fb_pitch;

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

  charbuf_init(f.pwidth, f.pheight);
  printf("Hello world!\n");
  charbuf_flush();

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
