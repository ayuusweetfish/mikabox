#ifndef _Mikabox_common_h_
#define _Mikabox_common_h_

#include <stdint.h>

#define mem_barrier() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")

#define PERI_BASE 0x20000000

#define GPFSEL4 (volatile uint32_t *)(PERI_BASE + 0x200000 + 0x10)
#define GPSET1  (volatile uint32_t *)(PERI_BASE + 0x200000 + 0x20)
#define GPCLR1  (volatile uint32_t *)(PERI_BASE + 0x200000 + 0x2c)

#define MAIL0_READ    (volatile uint32_t *)(PERI_BASE + 0xb880 + 0x00)
#define MAIL0_STATUS  (volatile uint32_t *)(PERI_BASE + 0xb880 + 0x18)
#define MAIL0_WRITE   (volatile uint32_t *)(PERI_BASE + 0xb880 + 0x20)

void send_mail(uint32_t data, uint8_t channel);
uint32_t recv_mail(uint8_t channel);

#endif
