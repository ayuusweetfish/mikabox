#include "mmu.h"

void mmu_section(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags)
{
  uint32_t *table_addr = (uint32_t *)((uint8_t *)table + (vaddr >> 18));
  uint32_t table_val = paddr | flags | 2; // 2 = section
  *table_addr = table_val;
}

void mmu_course_table(uint32_t *table, uint32_t vaddr, uint32_t *course_table, uint32_t flags)
{
  uint32_t *table_addr = (uint32_t *)((uint8_t *)table + (vaddr >> 18));
  uint32_t table_val = (uint32_t)course_table | flags | 1;  // 1 = course page table
  *table_addr = table_val;
}

void mmu_small_page(uint32_t *course_table, uint32_t vaddr, uint32_t paddr, uint32_t flags)
{
  // Section size is 2**20 bytes, page size is 2**12 bytes, entry size is 2**2 bytes
  // Offset is vaddr * (2**-12) * (2**2) = vaddr * (2**-10)
  uint32_t *table_addr = (uint32_t *)((uint8_t *)course_table + (vaddr >> 10));
  uint32_t table_val = paddr | flags | 2; // 2 = extended small page
  *table_addr = table_val;
}

static uint8_t ord_buf[1048576] __attribute__((section(".bss.ord")));
static size_t ord_ptr = 0;
static size_t ord_stack[2048], ord_stack_top = 0;

// Reentrancy is not required, since
// requests for strongly ordered memory cannot be dynamic
void *mmu_ord_alloc(size_t size, size_t align)
{
  size_t align_delta = (align - (ord_ptr % align)) % align;
  void *ret = ord_buf + ord_ptr + align_delta;
  ord_stack[ord_stack_top++] = size + align_delta;
  ord_ptr += size + align_delta;
  return ret;
}

void mmu_ord_pop()
{
  ord_ptr -= ord_stack[--ord_stack_top];
}
