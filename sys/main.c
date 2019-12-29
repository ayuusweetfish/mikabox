#include "main.h"
#include "common.h"
#include "charbuf.h"
#include "mmu.h"

#include "printf/printf.h"

#include <string.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;
extern unsigned char _bss_ord_begin;
extern unsigned char _bss_ord_end;

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
  for (uint8_t *p = &_bss_begin; p < &_bss_end; p++) *p = 0;
  for (uint8_t *p = &_bss_ord_begin; p < &_bss_ord_end; p++) *p = 0;

  uint32_t bss_ord_page_begin = (uint32_t)&_bss_ord_begin >> 20;
  uint32_t bss_ord_page_end = (uint32_t)(&_bss_ord_end - 1) >> 20;

  for (uint32_t i = 0; i < 4096; i++)
    mmu_table_section(mmu_table, i << 20, i << 20, (i < 64 ? (8 | 4) : 0));
  for (uint32_t i = bss_ord_page_begin; i <= bss_ord_page_end; i++)
    mmu_table_section(mmu_table, i << 20, i << 20, 0);
  mmu_enable(mmu_table);

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
  uint8_t count = 0;
  while (1) {
    mem_barrier();
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
  }
}
