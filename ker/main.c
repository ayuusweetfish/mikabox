#include "main.h"
#include "priv.h"
#include "regs.h"
#include "charbuf.h"
#include "mmu.h"
#include "irq.h"
#include "prop_tag.h"
#include "v3d.h"
#include "coroutine.h"
#include "swi.h"
#include "syscalls.h"

#include "printf/printf.h"
#include "ampi.h"
#include "ampienv.h"
#include "uspi.h"
#include "uspios.h"
#include "sdcard/sdcard.h"
#include "fatfs/ff.h"
#include "elf.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif

extern unsigned char _text_user_begin;
extern unsigned char _text_user_end;
extern unsigned char _bss_begin;
extern unsigned char _bss_end;
extern unsigned char _bss_ord_begin;
extern unsigned char _bss_ord_end;
extern unsigned char _kernel_end;
extern unsigned char _text_user_vaddr;

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

static uint8_t *fb_bufs[BUF_COUNT];
uint8_t fb_bufid = 0;
uint8_t *fb_buf;
uint32_t fb_pitch;

void fb_flip_buffer()
{
  set_virtual_offs(0, SCR_H * fb_bufid);
  fb_bufid = (fb_bufid + 1) % BUF_COUNT;
  fb_buf = fb_bufs[fb_bufid];
}

void (*periodic)() = NULL;
uint32_t uspi_tick = 0;
void uspi_upd_timers();

void timer2_callback(void *_unused)
{
  do *TMR_CS = 4; while (*TMR_CS & 4);
  uint32_t t = *TMR_CLO;
  t = t - t % 10000 + 10000;
  *TMR_C2 = t;

  if (periodic) periodic();
  uspi_tick++;
  uspi_upd_timers();
}

void vsync_callback(void *_unused)
{
  *(volatile uint32_t *)(PERI_BASE + 0x600000) = 0;
/*
  ctx.bufaddr = (uint32_t)fb_buf;
  v3d_op(&ctx);
  fb_flip_buffer();
  frame_count++;
*/
  charbuf_flush();
  fb_flip_buffer();
}

bool has_key = false;
static bool has_kbd_key = false, has_gpad_key = false;

static int16_t wavetable[256];

static unsigned synth(int16_t *buf, unsigned chunk_size)
{
  static uint8_t phase = 0;
  for (unsigned i = 0; i < chunk_size; i += 2) {
    int16_t sample = (has_key ? wavetable[phase] : 0);
    buf[i] = buf[i + 1] = sample;
    phase++;
  }
  return chunk_size;
}

static void kbd_upd_callback(uint8_t mod, const uint8_t k[6])
{
  printf("\r%02x %02x %02x %02x %02x %02x | mod = %02x",
    k[0], k[1], k[2], k[3], k[4], k[5], mod);
  has_kbd_key = (k[0] || k[1] || k[2] || k[3] || k[4] || k[5]);
  has_key = has_kbd_key | has_gpad_key;
}

static void gpad_upd_callback(unsigned index, const USPiGamePadState *state)
{
  printf("\r%d %08x", state->nbuttons, state->buttons);
  has_gpad_key = (state->buttons & 0x800);
  has_key = has_kbd_key | has_gpad_key;
  player_btns[0] = state->buttons;
}

static struct coroutine usb_co, audio_co;
static struct coroutine user_co[8];

static void usb_loop(uint32_t _unused)
{
  bool first = true;
  while (1) {
    if (first || USPiConnectionChanged()) {
      first = false;
      USPiDeinitialize();
      bool result = USPiInitialize();
      printf("USPi initialization %s\n", result ? "succeeded" : "failed");

      printf("Keyboard %savailable\n", USPiKeyboardAvailable() ? "" : "un");
      if (USPiKeyboardAvailable())
        USPiKeyboardRegisterKeyStatusHandlerRaw(kbd_upd_callback);

      printf("Gamepad %savailable\n", USPiGamePadAvailable() ? "" : "un");
      if (USPiGamePadAvailable())
        USPiGamePadRegisterStatusHandler(gpad_upd_callback);
    }
    MsDelay(100);
  }
}

static void audio_loop(uint32_t _unused)
{
  mem_barrier();
  AMPiInitialize(44100, 1764);  // 20 ms latency/block size
  AMPiSetChunkCallback(synth);
  bool b = AMPiStart();
  printf(b ? "Yes\n" : "No\n");

  while (1) {
    AMPiPoke();
    co_yield();
  }
}

static void file_get(void *user, void *dest, uint32_t offs, uint32_t len)
{
  FIL *f = (FIL *)user;
  UINT br;

  f_lseek(f, offs);
  f_read(f, dest, len, &br);

  if (len != br)
    printf("Read incomplete: offs = %u len = %u br = %u\n",
      offs, len, (unsigned)br);
}

static void *mem_map(elf_word vaddr, elf_word memsz, elf_word flags)
{
  uint32_t page_start = vaddr >> 20;
  uint32_t page_end = ((vaddr + memsz - 1) >> 20);
  for (uint32_t page = page_start; page <= page_end; page++) {
    uint32_t addr = page << 20;
    mmu_table_section(mmu_table, addr, addr - 0x40000000 + (32 << 20), (1 << 5) | (3 << 10) | (8 | 4));
  }

  mmu_flush();
  return (void *)vaddr;
}

uint32_t load_program(const char *path)
{
  FIL f;
  FRESULT r;

  if ((r = f_open(&f, path, FA_READ)) != FR_OK) {
    printf("f_open() returned error %d\n", (int)r);
    return 0;
  }

  elf_addr entry;
  uint8_t elfret = elf_load(file_get, mem_map, &f, &entry);
  f_close(&f);

  if (elfret != 0) {
    printf("elf_load() returned error %u\n", (unsigned)elfret);
    return 0;
  }
  return entry;
}

void sys_main()
{
  for (uint8_t *p = &_bss_begin; p < &_bss_end; p++) *p = 0;
  for (uint8_t *p = &_bss_ord_begin; p < &_bss_ord_end; p++) *p = 0;

  uint32_t bss_ord_page_begin = (uint32_t)&_bss_ord_begin >> 20;
  uint32_t bss_ord_page_end = (uint32_t)(&_bss_ord_end - 1) >> 20;

  uint32_t text_user_page = (uint32_t)&_text_user_vaddr >> 20;

  // Everything: sys RW, user N; cached up to 64 MiB
  for (uint32_t i = 0; i < 4096; i++)
    mmu_table_section(mmu_table, i << 20, i << 20, (i < 64 ? (8 | 4) : 0) | (1 << 5) | (1 << 10));
  // .bss.ord: sys RW, user N
  for (uint32_t i = bss_ord_page_begin; i <= bss_ord_page_end; i++)
    mmu_table_section(mmu_table, i << 20, i << 20, (1 << 5) | (1 << 10));
  // MMIO: sys RW, user N
  for (uint32_t i = 0x20000000; i <= 0x24000000; i += 0x100000)
    mmu_table_section(mmu_table, i, i, (1 << 5) | (1 << 10));
  // .text.user: sys RW, user R
  mmu_table_section(mmu_table, text_user_page << 20, text_user_page << 20, (1 << 5) | (2 << 10));
  // User region: sys RW, user RW
  for (uint32_t i = 0x40000000; i < 0x44000000; i += 0x100000)
    mmu_table_section(mmu_table, i, i - 0x40000000 + (32 << 20), (1 << 5) | (3 << 10) | (8 | 4));

  mmu_enable(mmu_table);
  // Client for domain 1, manager for domain 0
  mmu_domain_access_control((1 << 2) | 3);

  // Copy .text.user to separate page
  memcpy(&_text_user_vaddr, &_text_user_begin, &_text_user_end - &_text_user_begin);

  // Initialize RNG
  // https://github.com/bztsrc/raspi3-tutorial/blob/master/06_random/rand.c
  mem_barrier();
  *RNG_STATUS = 0x40000;
  *RNG_INTMASK |= 1;
  *RNG_CTRL |= 1;
  // Entropy starts accumulating

  mem_barrier();
  struct framebuffer *f = mmu_ord_alloc(sizeof(struct framebuffer), 16);
  memset(f, 0, sizeof(struct framebuffer));
  f->pwidth = SCR_W; f->pheight = SCR_H;
  f->vwidth = SCR_W; f->vheight = SCR_H * BUF_COUNT;
  f->bpp = 32;
  // 1: channel for framebuffer
  send_mail(((uint32_t)f + 0x40000000) >> 4, 1);
  recv_mail(1);

  uint8_t *base = (uint8_t *)(f->buf);
  fb_pitch = f->pitch;

  for (uint32_t i = 0; i < BUF_COUNT; i++)
    fb_bufs[i] = base + fb_pitch * f->pheight * i;
  fb_buf = fb_bufs[0];
  mmu_ord_pop();  // f

  mem_barrier();
  *TMR_CS = 4;
  *TMR_C2 = *TMR_CLO + 1000000;
  irq_set_callback(2, timer2_callback, NULL);

  mem_barrier();
  charbuf_init(SCR_W, SCR_H);
  printf("Hello world!\n");
  printf("ARM clock rate: %u\n", get_clock_rate(3));

  irq_set_callback(48, vsync_callback, NULL);

  //while (1) standby();

  sdInit();
  int32_t i = sdInitCard();
  printf("sdInitCard() returns %d\n", i);

  FATFS fs;
  FRESULT fr = f_mount(&fs, "", 1);
  printf("f_mount() returned %d\n", (int32_t)fr);

  // Continue RNG initialization
  mem_barrier();
  while (!((*RNG_STATUS) >> 24)) __asm__ __volatile__ ("");
  printf("Initialized HRNG\n");

  for (uint32_t i = 0; i < 256; i++) {
    wavetable[i] = (int16_t)(sin((double)i / 128 * M_PI * 2) * 32767);
  }

  mem_barrier();
  v3d_init();

  mem_barrier();
  syscalls_init();

  num_players = 1;
  player_btns[0] = 0;

/*
  void doda();
  void dodo(uint32_t);
  doda();
  while (1) {
    dodo((uint32_t)fb_buf);
    fb_flip_buffer();
  }
*/

  // USB and audio threads
  co_create(&usb_co, usb_loop);
  co_create(&audio_co, audio_loop);

  // Load overworld program
  uint32_t entry = load_program("/a.out");
  set_user_sp((void *)0x44000000);

  // Initialization routine
  co_create(&user_co[0], (void *)entry);
  user_co[0].flags = CO_FLAG_FPU | CO_FLAG_USER;

  mem_barrier();
  uint64_t app_start_time = ((uint64_t)*TMR_CHI << 32) | *TMR_CLO;
  routine_id = -1;

  while (routine_pc[0] == 0) {
    co_next(&usb_co);
    co_next(&audio_co);

    mem_barrier();
    uint64_t cur_time = ((uint64_t)*TMR_CHI << 32) | *TMR_CLO;
    app_tick = cur_time - app_start_time;
    co_next(&user_co[0]);
  }

  printf("Overworld initialized!\n");
  printf("routines: 0x%08x 0x%08x 0x%08x 0x%08x\n",
    routine_pc[0], routine_pc[1], routine_pc[2], routine_pc[3]);

  // Initialize coroutines
  for (uint32_t i = 0; i < 4; i++) {
    co_create(&user_co[i], (void *)routine_pc[i]);
    user_co[i].flags = CO_FLAG_FPU | CO_FLAG_USER;
  }

  // Main loop!
  while (1) {
    for (int i = 0; i < 500; i++) {
      co_next(&usb_co);
      co_next(&audio_co);
    }

    mem_barrier();
    uint64_t cur_time = ((uint64_t)*TMR_CHI << 32) | *TMR_CLO;
    app_tick = cur_time - app_start_time;
    //for (uint32_t i = 0; i < 4; i++)
    //  co_next(&user_co[i]);
    co_next(&user_co[0]);
  }

  while (1) { }
}
