#ifndef _Mikabox_charbuf_h_
#define _Mikabox_charbuf_h_

#include <stdint.h>

void charbuf_init(uint32_t width, uint32_t height);
void charbuf_flush();
void charbuf_invalidate();

void _putchar(char character);

#endif
