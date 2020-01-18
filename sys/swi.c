#include <stdint.h>
#include "printf/printf.h"

void swi_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
  register uint32_t num __asm__ ("r7");
  printf("%u %u %u %u %u\n", r0, r1, r2, r3, num);
}
