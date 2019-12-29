#include "mmu.h"

uint32_t mmu_table[4096] __attribute__((aligned(1 << 14)));

void mmu_table_section(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags)
{
  uint32_t *table_addr = (uint32_t *)((uint8_t *)table + (vaddr >> 18));
  uint32_t table_val = paddr | flags | 2; // 2 = section
  *table_addr = table_val;
}
