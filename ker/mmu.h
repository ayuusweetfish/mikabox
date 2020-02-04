#ifndef _Mikabox_mmu_h_
#define _Mikabox_mmu_h_

#include <stddef.h>
#include <stdint.h>

// table should be aligned to 2**14
// course_table should be aligned to 2**10

// Flags for section/course table: B4-27
// Flags for small page: B4-31

void mmu_section(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags);
void mmu_course_table(uint32_t *table, uint32_t vaddr, uint32_t *course_table, uint32_t flags);
void mmu_small_page(uint32_t *course_table, uint32_t vaddr, uint32_t paddr, uint32_t flags);
void mmu_enable(uint32_t *table_base);
void mmu_flush();

void mmu_domain_access_control(uint32_t access_vector);
uint32_t mmu_domain_access_control_get();

void *mmu_ord_alloc(size_t size, size_t align);
void mmu_ord_pop();

#endif
