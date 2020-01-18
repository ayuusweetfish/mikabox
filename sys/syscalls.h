#include <stdint.h>

#if SYSCALLS_DECL
  #define def(__id, __fn)   \
    void syscall_handler_##__id(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
#elif SYSCALLS_IMPL
  #define def(__id, __fn)   \
    void syscall_handler_##__id(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) __fn
#elif SYSCALLS_TABLE
  #define def(__id, __fn)   [__id] = &syscall_handler_##__id,
#else
  #define def(__id, __fn)
#endif

#if SYSCALLS_IMPL
#include "printf/printf.h"
#endif

def(43, {
  printf("from syscall! %u\n", r0);
})

#undef impl
#undef def
