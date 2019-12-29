#ifndef _Mikabox_irq_h_
#define _Mikabox_irq_h_

#include <stdint.h>

typedef void (*irq_callback_t)();
void irq_set_callback(uint8_t source, irq_callback_t fn);

#endif
