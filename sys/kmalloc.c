#include "kmalloc.h"

#include <stdint.h>

// TODO: Allow finer control over alignment
// and memory attributes, maybe unify mmu_ord_alloc/pop

static uint8_t buf[1048576];
static size_t ptr = 0;

void uspi_EnterCritical();
void uspi_LeaveCritical();

void *kmalloc(size_t size)
{
  uspi_EnterCritical();
  size = (size + 15) & ~15;
  ptr += size;
  void *ret = buf + (ptr - size);
  uspi_LeaveCritical();
  return ret;
}

void kfree(void *ptr)
{
}

void kmalloc_reset()
{
  ptr = 0;
}
