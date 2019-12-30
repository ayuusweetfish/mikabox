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

#define IRQ_PENDBASIC (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x200)
#define IRQ_PEND1     (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x204)
#define IRQ_PEND2     (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x208)
#define IRQ_ENAB1     (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x210)
#define IRQ_ENAB2     (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x214)
#define IRQ_ENABBASIC (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x218)
#define IRQ_DISA1     (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x21c)
#define IRQ_DISA2     (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x220)
#define IRQ_DISABASIC (volatile uint32_t *)(PERI_BASE + 0xb000 + 0x224)

#define TMR_CS  (volatile uint32_t *)(PERI_BASE + 0x3000 + 0x00)
#define TMR_CLO (volatile uint32_t *)(PERI_BASE + 0x3000 + 0x04)
#define TMR_C0  (volatile uint32_t *)(PERI_BASE + 0x3000 + 0x0c)
#define TMR_C1  (volatile uint32_t *)(PERI_BASE + 0x3000 + 0x10)
#define TMR_C2  (volatile uint32_t *)(PERI_BASE + 0x3000 + 0x14)
#define TMR_C3  (volatile uint32_t *)(PERI_BASE + 0x3000 + 0x18)

#define V3D_IDENT0  (volatile uint32_t *)(PERI_BASE + 0xc00000 + 0x000)

void send_mail(uint32_t data, uint8_t channel);
uint32_t recv_mail(uint8_t channel);

#endif
