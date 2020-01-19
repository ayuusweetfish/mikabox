#include "swi.h"
#include "printf/printf.h"

#define SYSCALLS_DECL 1
#include "syscalls.h"
#undef SYSCALLS_DECL

#include <stdint.h>

typedef uint32_t (*syscall_fn_t)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);

#define SYSCALLS_TABLE 1

static const syscall_fn_t general[4096] = {
  #include "syscalls.h"
};

static const syscall_fn_t priv[4096] = {
};

uint32_t swi_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
  register uint32_t r7 __asm__ ("r7");
  uint32_t num = r7;

  syscall_fn_t fn;
  if (num & 0x10000) {
    fn = priv[num & 4095];
  } else {
    fn = general[num & 4095];
  }
  uint32_t ret = 0;
  if (fn != NULL) ret = (*fn)(r0, r1, r2, r3);
  return ret;
}

// Calls
// https://gcc.gnu.org/onlinedocs/gcc/Modifiers.html
// https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html

#define syscall_attr //__attribute__((noinline))

syscall_attr uint32_t syscall0(uint32_t num)
{
  return syscall4(num, 0, 0, 0, 0);
}

syscall_attr uint32_t syscall1(uint32_t num, uint32_t arg0)
{
  return syscall4(num, arg0, 0, 0, 0);
}

syscall_attr uint32_t syscall2(uint32_t num, uint32_t arg0, uint32_t arg1)
{
  return syscall4(num, arg0, arg1, 0, 0);
}

syscall_attr uint32_t syscall3(uint32_t num, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
  return syscall4(num, arg0, arg1, arg2, 0);
}

syscall_attr uint32_t syscall4(uint32_t num, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
  register uint32_t r0 __asm__ ("r0");
  __asm__ __volatile__ (
    "mov r7, %[num]\n"
    "mov r0, %[arg0]\n"
    "mov r1, %[arg1]\n"
    "mov r2, %[arg2]\n"
    "mov r3, %[arg3]\n"
    "swi #0\n"
    : "=&r" (r0)
    : [num] "r" (num), [arg0] "r" (arg0), [arg1] "r" (arg1), [arg2] "r" (arg2), [arg3] "r" (arg3)
    : "r7", "lr", "memory", "r1", "r2", "r3"
  );
  return r0;
}

