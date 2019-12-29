#include "kmalloc.h"

#include <stdint.h>

static uint8_t buf[1048576];
static size_t ptr = 0;

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
