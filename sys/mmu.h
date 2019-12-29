#ifndef _Mikabox_mmu_h_
#define _Mikabox_mmu_h_

#include <stdint.h>

extern uint32_t mmu_table[4096];

void mmu_table_section(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags);
void mmu_enable(uint32_t *table_base);
void mmu_flush();

#endif
