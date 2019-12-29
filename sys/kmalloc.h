#ifndef _Mikabox_kmalloc_h_
#define _Mikabox_kmalloc_h_

#include <stddef.h>

void *kmalloc(size_t size);
void kfree(void *ptr);
// Debug use
void kmalloc_reset();

#endif
