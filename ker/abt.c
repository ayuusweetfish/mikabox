#include "printf/printf.h"

#include <stdint.h>

__attribute__ ((interrupt("ABORT")))
void _dabt_stub()
{
  register uint32_t lr_reg __asm__ ("lr");
  uint32_t lr = lr_reg;

  printf("Data abort at 0x%08x\n", lr);
  __asm__ __volatile__ ("cpsie i");
  while (1) { }
}

__attribute__ ((interrupt("ABORT")))
void _pfabt_stub()
{
  register uint32_t lr_reg __asm__ ("lr");
  uint32_t lr = lr_reg;

  printf("Prefetch abort at 0x%08x\n", lr);
  __asm__ __volatile__ ("cpsie i");
  while (1) { }
}
