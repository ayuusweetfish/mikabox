#include "mmu.h"

uint32_t mmu_table[4096] __attribute__((aligned(1 << 14)));

void mmu_table_section(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags)
{
  uint32_t *table_addr = (uint32_t *)((uint8_t *)table + (vaddr >> 18));
  uint32_t table_val = paddr | flags | 2; // 2 = section
  *table_addr = table_val;
}

static uint8_t ord_buf[1048576] __attribute__((section(".bss.ord")));
static size_t ord_ptr = 0;
static size_t ord_stack[2048], ord_stack_top = 0;

// Reentrancy is not required, since
// requests for strongly ordered memory cannot be dynamic
void *mmu_ord_alloc(size_t size)
{
  void *ret = ord_buf + ord_ptr;
  ord_stack[ord_stack_top++] = size;
  ord_ptr += size;
  return ret;
}

void mmu_ord_pop()
{
  ord_ptr -= ord_stack[--ord_stack_top];
}
