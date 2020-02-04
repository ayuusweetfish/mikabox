#include "printf/printf.h"
#include "charbuf.h"

#include <stdbool.h>
#include <stdint.h>

bool is_in_abt = false;

__attribute__ ((interrupt("ABORT")))
void _dabt_stub()
{
  register uint32_t lr_reg __asm__ ("lr");
  uint32_t lr = lr_reg - 4;

  is_in_abt = true;
  charbuf_invalidate();
  __asm__ __volatile__ ("cpsie i");
  printf("Data abort at 0x%08x\n", lr);
  while (1) { }
}

__attribute__ ((interrupt("ABORT")))
void _pfabt_stub()
{
  register uint32_t lr_reg __asm__ ("lr");
  uint32_t lr = lr_reg - 4;

  is_in_abt = true;
  charbuf_invalidate();
  __asm__ __volatile__ ("cpsie i");
  printf("Prefetch abort at 0x%08x\n", lr);
  while (1) { }
}
