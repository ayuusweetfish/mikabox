#include "kmalloc.h"

#include <stdint.h>

static uint8_t buf[6553600];
static size_t ptr = 1;  // Change to 0 after .bss is handled

void *kmalloc(size_t size)
{
  ptr += size;
  return buf + (ptr - size);
}

void kfree(void *ptr)
{
}

void kmalloc_reset()
{
  ptr = 0;
}
