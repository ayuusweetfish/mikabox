#include <stdint.h>

#define FN(__grp, __id)    \
  syscall_##__grp##_##__id

#if SYSCALLS_DECL
  #define def(__grp, __id, __fn)   \
    void FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
#elif SYSCALLS_IMPL
  #define def(__grp, __id, __fn)   \
    void FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) __fn
#elif SYSCALLS_TABLE
  #define def(__grp, __id, __fn)   \
    [SYSCALL_GRP_OFFS_##__grp + __id] = &FN(__grp, __id),
#else
  #define def(__grp, __id, __fn)
#endif

#if SYSCALLS_IMPL
#include "printf/printf.h"
#endif

#define SYSCALL_GRP_OFFS_GEN  0
#define SYSCALL_GRP_OFFS_GFX  256

def(GEN, 43, {
  printf("from syscall! %u\n", r0);
})

def(GFX, 0, {
  printf("graphics!\n");
})

#undef impl
#undef def
#undef FN
