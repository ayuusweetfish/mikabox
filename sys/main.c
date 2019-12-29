#include "main.h"
#include "common.h"
#include "charbuf.h"
#include "mmu.h"
#include "irq.h"

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

#define BUF_COUNT 2
static uint8_t *fb_bufs[BUF_COUNT];
static uint32_t fb_bufid = 0;
uint8_t *fb_buf;
uint32_t fb_pitch;

uint32_t get_clock_rate(uint8_t id)
{
  prop_tag_8 *buf = mmu_ord_alloc(sizeof(prop_tag_8), 16);
  prop_tag_init(buf);
  buf->tag.id = 0x30002;
  buf->tag.u32[0] = id;
  prop_tag_emit(buf);
  uint32_t ret = buf->tag.u32[1];
  mmu_ord_pop();
  return ret;
}

void set_virtual_offs(uint32_t x, uint32_t y)
{
  prop_tag_8 *buf = mmu_ord_alloc(sizeof(prop_tag_8), 16);
  prop_tag_init(buf);
  buf->tag.id = 0x48009;
  buf->tag.u32[0] = x;
  buf->tag.u32[1] = y;
  prop_tag_emit(buf);
  mmu_ord_pop();
}

void fb_flip_buffer()
{
  set_virtual_offs(0, 480 * fb_bufid);
  fb_bufid = (fb_bufid + 1) % BUF_COUNT;
  fb_buf = fb_bufs[fb_bufid];
}

void timer3_callback()
{
  do *TMR_CS = 8; while (*TMR_CS & 8);
  uint32_t t = *TMR_CLO;
  t = t - t % 1000000 + 1000000;
  *TMR_C3 = t;

  printf("Timer!\n");
}

void vsync_callback()
{
  *(volatile uint32_t *)(PERI_BASE + 0x600000) = 0;
  static uint8_t count = 0;
  if (++count == 4) {
    charbuf_flush();
    fb_flip_buffer();
    count = 0;
  }
}

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

  *TMR_CS = 8;
  *TMR_C3 = 6000000;
  irq_set_callback(3, timer3_callback);
  irq_set_callback(48, vsync_callback);

  struct framebuffer *f = mmu_ord_alloc(sizeof(struct framebuffer), 16);
  memset(f, 0, sizeof(struct framebuffer));
  f->pwidth = 800; f->pheight = 480;
  f->vwidth = 800; f->vheight = 480 * BUF_COUNT;
  f->bpp = 24;
  // 1: channel for framebuffer
  send_mail(((uint32_t)f + 0x40000000) >> 4, 1);
  recv_mail(1);

  uint8_t *base = (uint8_t *)(f->buf);
  fb_pitch = f->pitch;

  for (uint32_t i = 0; i < BUF_COUNT; i++)
    fb_bufs[i] = base + fb_pitch * f->pheight * i;
  fb_buf = fb_bufs[0];

  charbuf_init(f->pwidth, f->pheight);
  printf("Hello world!\n");
  printf("ARM clock rate: %u\n", get_clock_rate(3));
  mmu_ord_pop();  // f

  mem_barrier();
  *GPFSEL4 |= (1 << 21);
  uint8_t count = 99;
  while (count > 0) {
    printf("\n");
    printf("%u bottle%s of beer on the wall\n", count, count == 1 ? "" : "s");
    printf("%u bottle%s of beer\n", count, count == 1 ? "" : "s");

    mem_barrier();
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");

    printf("Take one down, pass it around\n");
    count--;
    printf("%u bottle%s of beer on the wall\n", count, count == 1 ? "" : "s");

    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
  }
}
