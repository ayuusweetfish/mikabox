#include <stdint.h>

#define GPIO_BASE 0x20200000
#define GPFSEL4   (volatile uint32_t *)(GPIO_BASE + 0x10)
#define GPSET1    (volatile uint32_t *)(GPIO_BASE + 0x20)
#define GPCLR1    (volatile uint32_t *)(GPIO_BASE + 0x2c)

void sys_main()
{
  *GPFSEL4 |= (1 << 21);
  while (1) {
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
  }
}
