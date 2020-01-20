#include "swi.h"
#include "printf/printf.h"
#include "coroutine.h"
#include "sys.h"

#define SYSCALLS_DECL 1
#include "syscalls.h"
#undef SYSCALLS_DECL

#include <stdint.h>

typedef uint64_t (*syscall_fn_t)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);

#define SYSCALLS_TABLE 1

static const syscall_fn_t general[4096] = {
  #include "syscalls.h"
};

static const syscall_fn_t priv[4096] = {
};

struct reg_set user_yield_regs;

__attribute__((noinline))
uint64_t swi_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
  register uint32_t r7 __asm__ ("r7");
  uint32_t num = r7;

  syscall_fn_t fn;
  if (num & 0x10000) {
    fn = priv[num & 4095];
  } else {
    fn = general[num & 4095];
  }

  if (num == 1) {
    user_yield_regs.sp = (uint32_t)get_user_sp();
    printf("saved stack pointer: 0x%08x\n", user_yield_regs.sp);
    printf("return address: 0x%08x\n", user_yield_regs.pc);
    co_syscall_yield(&user_yield_regs);
  }

  if (fn != NULL)
    return (*fn)(r0, r1, r2, r3);
  else return 0;
}
