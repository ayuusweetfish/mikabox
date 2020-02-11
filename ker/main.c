#include "main.h"
#include "priv.h"
#include "regs.h"
#include "charbuf.h"
#include "mmu.h"
#include "irq.h"
#include "prop_tag.h"
#include "v3d.h"
#include "audio.h"
#include "coroutine.h"
#include "swi.h"
#include "syscalls.h"
#include "input.h"

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

uint32_t mmu_table[4096] __attribute__ ((aligned(1 << 14)));
uint32_t mmu_course_o[64][256] __attribute__ ((aligned(1 << 10)));
uint32_t mmu_course_a[64][256] __attribute__ ((aligned(1 << 10)));

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

extern bool is_in_abt;
static bool flipped = false;
static bool input_updated = false;

static uint64_t player_btns_last[MAX_PLAYERS];

static inline void enable_charbuf()
{
  charbuf_invalidate();
  is_in_abt = true;
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
  flipped = true;
  if (!is_in_abt && app_fb_buf != 0) return;
  if (is_in_abt) charbuf_flush();
  fb_flip_buffer();
}

static void kbd_upd_callback(uint8_t mod, const uint8_t k[6])
{
  //printf("\r%02x %02x %02x %02x %02x %02x | mod = %02x",
  //  k[0], k[1], k[2], k[3], k[4], k[5], mod);
  input_updated = true;
}

static void gpad_upd_callback(unsigned index, const USPiGamePadState *state)
{
  //memcpy(player_btns_last, player_btns, sizeof player_btns);

  //printf("\r%d %08x", state->nbuttons, state->buttons);
  unsigned b = state->buttons;
  unsigned dp = state->hats[0];
  if (state->type == USBGamePadTypePS4) {
    player_btns[0] =
      BTN_BIT(SQR, b, 0) |
      BTN_BIT(CRO, b, 1) |
      BTN_BIT(CIR, b, 2) |
      BTN_BIT(TRI, b, 3) |
      BTN_BIT(L1, b, 4) |
      BTN_BIT(R1, b, 5) |
      BTN_BIT(L2, b, 6) |
      BTN_BIT(R2, b, 7) |
      BTN_BIT(L3, b, 10) |
      BTN_BIT(R3, b, 11) |
      BTN_BIT(START, b, 8) |
      BTN_BIT(OPTN, b, 9) |
      BTN_BIT(META, b, 12) |
      BTN_BIT(AUX, b, 13);
    uint32_t dp_btns = 0;
    if (dp == 7 || dp <= 1) dp_btns |= BTN_U;
    if (dp >= 1 && dp <= 3) dp_btns |= BTN_R;
    if (dp >= 3 && dp <= 5) dp_btns |= BTN_D;
    if (dp >= 5 && dp <= 7) dp_btns |= BTN_L;
    player_btns[0] |= dp_btns;
    //printf("%u %08x\n", index, player_btns[0]);
  }
  input_updated = true;
}

static struct coroutine usb_co, audio_co;
static uint8_t usb_co_stack[65536] __attribute__ ((aligned(8)));
static uint8_t audio_co_stack[65536] __attribute__ ((aligned(8)));

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
  AMPiInitialize(44100, audio_blocksize() * 2);
  AMPiSetChunkCallback(audio_callback);
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
  uint32_t page_start = vaddr >> 12;
  uint32_t page_end = ((vaddr + memsz - 1) >> 12);
  for (uint32_t page = page_start; page <= page_end; page++) {
    uint32_t addr = (page << 12) - 0x40000000;
    mmu_small_page(&mmu_course_o[0][0], addr, addr + (32 << 20), (8 | 4) | (3 << 4));
  }

  mmu_flush();
  return (void *)vaddr;
}

static struct app_timer {
  uint64_t base;
  uint64_t start;
} timer_g = { 0 }, timer_o = { 0 }, timer_a = { 0 };

void app_timer_start(struct app_timer *t)
{
  mem_barrier();
  uint64_t cur_time = ((uint64_t)*TMR_CHI << 32) | *TMR_CLO;
  t->start = cur_time;
}

uint64_t app_timer_update(struct app_timer *t)
{
  mem_barrier();
  uint64_t cur_time = ((uint64_t)*TMR_CHI << 32) | *TMR_CLO;
  return cur_time - t->start + t->base;
}

void app_timer_pause(struct app_timer *t)
{
  t->base = app_timer_update(t);
}

bool load_program(const char *path, bool overworld)
{
  FIL f;
  FRESULT r;

  if ((r = f_open(&f, path, FA_READ)) != FR_OK) {
    printf("f_open() returned error %d\n", (int)r);
    return false;
  }

  elf_addr entry;
  uint8_t elfret = elf_load(file_get, mem_map, &f, &entry);
  f_close(&f);

  if (elfret != 0) {
    printf("elf_load() returned error %u\n", (unsigned)elfret);
    return false;
  }

  uint32_t bank = (overworld ? 0 : 4);
  uint32_t memory_end = (overworld ? 0x44000000 : 0x84000000);

  // Initialization routine
  co_create(&user_co[bank], (void *)entry,
    CO_FLAG_FPU | CO_FLAG_USER, (void *)memory_end);

  mem_barrier();
  routine_id = (overworld ? -1 : -2);

  while (routine_pc[bank] == 0) {
    co_next(&usb_co);
    co_next(&audio_co);

    mem_barrier();
    co_next(&user_co[bank]);
  }

  // Initialize coroutines
  for (uint32_t i = 0; i < 4; i++) {
    co_create(&user_co[bank + i], (void *)routine_pc[bank + i],
      CO_FLAG_FPU | CO_FLAG_USER, (void *)(memory_end - 0x100000 * i));
  }

  return true;
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
    mmu_section(mmu_table, i << 20, i << 20, (i < 64 ? (8 | 4) : 0) | (1 << 5) | (1 << 10));
  // .bss.ord: sys RW, user N
  for (uint32_t i = bss_ord_page_begin; i <= bss_ord_page_end; i++)
    mmu_section(mmu_table, i << 20, i << 20, (1 << 5) | (1 << 10));
  // MMIO: sys RW, user N
  for (uint32_t i = 0x20000000; i <= 0x24000000; i += 0x100000)
    mmu_section(mmu_table, i, i, (1 << 5) | (1 << 10));
  // .text.user: sys RW, user R
  mmu_section(mmu_table, text_user_page << 20, text_user_page << 20, (1 << 5) | (2 << 10));
  // User region: sys RW, user RW
  for (uint32_t i = 0x40000000; i < 0x44000000; i += 0x100000)
    mmu_course_table(mmu_table, i, mmu_course_o[(i - 0x40000000) >> 20], (1 << 5));
  for (uint32_t i = 0; i < 0x4000000; i += 0x1000)  // Needed for stack space
    mmu_small_page(&mmu_course_o[0][0], i, i + (32 << 20), (8 | 4) | (3 << 4));

  for (uint32_t i = 0x80000000; i < 0x84000000; i += 0x100000)
    mmu_course_table(mmu_table, i, mmu_course_a[(i - 0x80000000) >> 20], (1 << 5));
  for (uint32_t i = 0; i < 0x4000000; i += 0x1000)  // Needed for stack space
    mmu_small_page(&mmu_course_a[0][0], i, i + (128 << 20), (8 | 4) | (3 << 4));

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

/*
  volatile uint32_t *p = (uint32_t *)0x40123abc;
  volatile uint32_t *q = (uint32_t *)((32 << 20) + 0x123abc);
  *p = 0xbaadaa;
  printf("%08x %08x\n", *p, *q);
  enable_charbuf();
  while (1) standby();
*/

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

  app_timer_start(&timer_g);

  // USB and audio threads
  co_create(&usb_co, usb_loop, 0, usb_co_stack + sizeof usb_co_stack);
  co_create(&audio_co, audio_loop, 0, audio_co_stack + sizeof audio_co_stack);

  //enable_charbuf();
  //MsDelay(2500);

  // Load overworld program
  bool load_result = load_program("/a.out", true);

  printf("Overworld initialized!\n");
  printf("routines: 0x%08x 0x%08x 0x%08x 0x%08x\n",
    routine_pc[0], routine_pc[1], routine_pc[2], routine_pc[3]);
  printf("Memory: %llx + %llx\n", get_arm_memory(), get_gpu_memory());

  uint64_t last_comp = 0;
  uint64_t next_draw_req = (uint64_t)-1;
  req_flags = 0xf;

  //while (1) { }
  app_timer_start(&timer_o);

  // Main loop!
  while (1) {
    co_next(&usb_co);
    co_next(&audio_co);

    if (!program_paused && !(player_btns_last[0] & BTN_START) && (player_btns[0] & BTN_START)) {
      // Pause
      program_paused = true;
      app_timer_pause(&timer_a);
      app_timer_start(&timer_o);
    } else if (program_name[0] != '\0' && request_resume) {
      // Resume
      request_resume = false;
      program_paused = false;
      app_timer_pause(&timer_o);
      app_timer_start(&timer_a);
    }

    int bank = ((program_name[0] == '\0' || program_paused) ? 0 : 4);
    struct app_timer *timer = (bank == 0 ? &timer_o : &timer_a);

    for (int8_t i = 3; i >= 0; i--) if (req_flags & (1 << i)) {
      mem_barrier();
      routine_id = bank + i;
      app_tick = app_timer_update(timer);
      if (i == 0) app_fb_buf = (uint32_t)fb_buf;
      co_next(&user_co[bank + i]);
      if (i == 0) app_fb_buf = 0;
    }

    uint64_t global_tick = app_timer_update(&timer_g);
    //printf("%8llu %8llu\n", global_tick, app_tick);

    if (request_exec[0] != '\0') {
      printf("execute: %s\n", request_exec);
      load_program(request_exec, false);
      printf("routines: 0x%08x 0x%08x 0x%08x 0x%08x\n",
        routine_pc[4], routine_pc[5], routine_pc[6], routine_pc[7]);

      request_exec[0] = '\0';
      strncpy(program_name, "Some Program", sizeof(program_name) - 1);
      program_name[sizeof(program_name) - 1] = '\0';
      program_paused = false;
      app_timer_pause(&timer_o);
      app_timer_start(&timer_a);
    }

    if (flipped) {
      flipped = false;
      next_draw_req = global_tick + 2000;
    }
    if (next_draw_req < global_tick) {
      next_draw_req = (uint64_t)-1;
      req_flags |= (1 << 0);
    }

    if (audio_pending()) {
      req_flags |= (1 << 1);
    }

    if (input_updated) {
      input_updated = false;
      req_flags |= (1 << 2);
    }

    if (last_comp != global_tick * 240 / 1000000) {
      last_comp = global_tick * 240 / 1000000;
      req_flags |= (1 << 3);
    }
  }

  while (1) { }
}
