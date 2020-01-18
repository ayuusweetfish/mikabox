#ifndef _Mikabox_swi_h_
#define _Mikabox_swi_h_

// https://gcc.gnu.org/onlinedocs/gcc/Modifiers.html
// https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html

static inline uint32_t syscall0(uint32_t num)
{
  register uint32_t r0 __asm__ ("r0");
  __asm__ __volatile__ (
    "mov r7, %[num]\n"
    "swi #0\n"
    : "=&r,&r" (r0)     // Should always be register r0
    : [num] "r,I" (num) // General register, or (8 bit rot even) immediate
    : "r7", "lr", "memory", "r1", "r2", "r3"
  );
  return r0;
}

static inline uint32_t syscall1(uint32_t num, uint32_t arg0)
{
  register uint32_t r0 __asm__ ("r0");
  __asm__ __volatile__ (
    "mov r7, %[num]\n"
    "mov r0, %[arg0]\n"
    "swi #0\n"
    : "=&r,&r" (r0)
    : [num] "r,I" (num), [arg0] "r,I" (arg0)
    : "r7", "lr", "memory", "r1", "r2", "r3", "r4"
  );
  return r0;
}

static inline uint32_t syscall2(uint32_t num, uint32_t arg0, uint32_t arg1)
{
  register uint32_t r0 __asm__ ("r0");
  __asm__ __volatile__ (
    "mov r7, %[num]\n"
    "mov r0, %[arg0]\n"
    "mov r1, %[arg1]\n"
    "swi #0\n"
    : "=&r,&r" (r0)
    : [num] "r,I" (num), [arg0] "r,I" (arg0), [arg1] "r,I" (arg1)
    : "r7", "lr", "memory", "r1", "r2", "r3"
  );
  return r0;
}

static inline uint32_t syscall3(uint32_t num, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
  register uint32_t r0 __asm__ ("r0");
  __asm__ __volatile__ (
    "mov r7, %[num]\n"
    "mov r0, %[arg0]\n"
    "mov r1, %[arg1]\n"
    "mov r2, %[arg2]\n"
    "swi #0\n"
    : "=&r,&r" (r0)
    : [num] "r,I" (num), [arg0] "r,I" (arg0), [arg1] "r,I" (arg1), [arg2] "r,I" (arg2)
    : "r7", "lr", "memory", "r1", "r2", "r3"
  );
  return r0;
}

static inline uint32_t syscall4(uint32_t num, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
  register uint32_t r0 __asm__ ("r0");
  __asm__ __volatile__ (
    "mov r7, %[num]\n"
    "mov r0, %[arg0]\n"
    "mov r1, %[arg1]\n"
    "mov r2, %[arg2]\n"
    "mov r3, %[arg3]\n"
    "swi #0\n"
    : "=&r,&r" (r0)
    : [num] "r,I" (num), [arg0] "r,I" (arg0), [arg1] "r,I" (arg1), [arg2] "r,I" (arg2), [arg3] "r,I" (arg3)
    : "r7", "lr", "memory", "r1", "r2", "r3"
  );
  return r0;
}

uint32_t swi_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);

#endif
