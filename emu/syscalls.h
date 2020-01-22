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

#if SYSCALLS_DECL
void syscalls_init();

#elif SYSCALLS_IMPL
#include "../sys/pool.h"
#include <stdio.h>
#include <string.h>

#define syscall_log(_fmt, ...) \
  printf("%s: " _fmt, __func__, ##__VA_ARGS__)

void syscall_read_mem(uint32_t addr, uint32_t size, void *buf);
#endif

def(GEN, 1, {
})

def(GEN, 6, {
  printf("(r1, r0) = 0x%08x%08x\n", r1, r0);
  uint32_t data = 0xbaa;
  return (uint64_t)data | 0x100000000LL;
})

def(GEN, 43, {
  printf("from syscall! 0x%08x\n", r0);
  char ch;
  while (1) {
    syscall_read_mem(r0++, 1, &ch);
    if (ch == '\0') break;
    putchar(ch);
  }
  putchar('\n');
})

#undef def
#undef init
#undef FN

#undef SYSCALLS_DECL
#undef SYSCALLS_IMPL
#undef SYSCALLS_TABLE
#undef SYSCALLS_INIT
