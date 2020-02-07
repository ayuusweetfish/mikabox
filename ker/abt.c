#include "printf/printf.h"
#include "charbuf.h"
#include "regs.h"

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

  *GPFSEL4 |= (1 << 21);
  while (1) {
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
  }
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

  *GPFSEL4 |= (1 << 21);
  while (1) {
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
  }
}
