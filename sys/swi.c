#include <stdint.h>
#include "printf/printf.h"

#define SYSCALLS_DECL 1
#include "syscalls.h"
#undef SYSCALLS_DECL

typedef void (*syscall_fn_t)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);

#define SYSCALLS_TABLE 1

static const syscall_fn_t general[4096] = {
  #include "syscalls.h"
};

static const syscall_fn_t priv[4096] = {
};

void swi_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
  register uint32_t r7 __asm__ ("r7");
  uint32_t num = r7;
  printf("%u %u %u %u %u\n", num, r0, r1, r2, r3);

  syscall_fn_t fn;
  if (num & 0x10000) {
    fn = priv[num & 0xffff];
  } else {
    fn = general[num];
  }
  if (fn != NULL) (*fn)(r0, r1, r2, r3);
}
