#include <stdint.h>

#define FN(__grp, __id)    \
  syscall_##__grp##_##__id

#define init(__fn)

#if !SYSCALLS_DECL && !SYSCALLS_IMPL && !SYSCALLS_TABLE && !SYSCALLS_INIT
#define SYSCALLS_DECL 1
#endif

#if SYSCALLS_DECL
  #define def(__grp, __id, __fn)  \
    uint64_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
#elif SYSCALLS_IMPL
  #define def(__grp, __id, __fn)  \
    uint64_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) \
    { __fn return 0; }
#elif SYSCALLS_TABLE
  #define def(__grp, __id, __fn)  \
    [SYSCALL_GRP_OFFS_##__grp + __id] = &FN(__grp, __id),
#elif SYSCALLS_INIT
  #define def(__grp, __id, __fn)
  #undef init
  #define init(__fn)  __fn
#else
  #define def(__grp, __id, __fn)
#endif

#define SYSCALL_GRP_OFFS_GEN  0
#define SYSCALL_GRP_OFFS_GFX  256
#define SYSCALL_GRP_OFFS_FIL  512
#define SYSCALL_GRP_OFFS_AUD  768

#if SYSCALLS_DECL
void syscalls_init();

#elif SYSCALLS_IMPL
#include "emu.h"
#include "../sys/pool.h"
#include <stdio.h>
#include <string.h>

#define syscall_log(_fmt, ...) \
  printf("%s: " _fmt, __func__, ##__VA_ARGS__)

void syscall_read_mem(uint32_t addr, uint32_t size, void *buf);
void syscall_reinit_rng();
uint64_t syscalls_lcg;
static uint32_t rng_count = 0;

#endif

def(GEN, 0, {
  if (routine_id != -1) {
    syscall_log("Routines can only be chaned in initialization routine\n");
    return 0;
  }
  routine_pc[0] = r0;
  routine_pc[1] = r1;
  routine_pc[2] = r2;
})

def(GEN, 1, {
})

def(GEN, 2, {
  update_tick();
  return app_tick;
})

def(GEN, 3, {
  return num_players;
})

def(GEN, 4, {
  if (r0 < num_players)
    return player_btns[r0];
})

def(GEN, 5, {
  if (r0 < num_players)
    return player_axes[r0];
})

def(GEN, 6, {
  if (++rng_count >= 1e6) {
    rng_count = 0;
    syscall_reinit_rng();
  }
  // Newlib/Musl LCG implementation
  uint64_t ret;
  syscalls_lcg = (syscalls_lcg * 6364136223846793005LL + 1);
  ret = (syscalls_lcg >> 32) << 32;
  syscalls_lcg = (syscalls_lcg * 6364136223846793005LL + 1);
  ret = ret | (syscalls_lcg >> 32);
  return ret;
})

def(GEN, 7, {
  char last;
  char ch;
  while (1) {
    syscall_read_mem(r0++, 1, &ch);
    if (ch == '\0') break;
    putchar(last = ch);
  }
  if (last != '\n') putchar('\n');
})

#undef def
#undef init
#undef FN

#undef SYSCALLS_DECL
#undef SYSCALLS_IMPL
#undef SYSCALLS_TABLE
#undef SYSCALLS_INIT
