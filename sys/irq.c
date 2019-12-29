#include "irq.h"
#include "common.h"
#include <stddef.h>

#define IRQ_MAX 72
static irq_callback_t callbacks[IRQ_MAX] = { NULL };

void irq_handler(uint32_t ret_addr)
{
  // Check interrupt source
  mem_barrier();
  uint32_t pend_base = *IRQ_PENDBASIC;
  uint8_t source;
  if (pend_base & (1 << 8)) {
    source = 0 + __builtin_ctz(*IRQ_PEND1);
  } else if (pend_base & (1 << 9)) {
    source = 32 + __builtin_ctz(*IRQ_PEND2);
  } else if (pend_base & 0xff) {
    source = 64 + __builtin_ctz(pend_base & 0xff);
  } else {
    // Should not reach here
    return;
  }
  mem_barrier();

  if (callbacks[source]) callbacks[source]();
}

void irq_set_callback(uint8_t source, irq_callback_t fn)
{
  mem_barrier();
  if (source >= IRQ_MAX) return;
  if (fn != NULL) {
    if (source < 32) *IRQ_ENAB1 = (1 << source);
    else if (source < 64) *IRQ_ENAB2 = (1 << (source - 32));
    else *IRQ_ENABBASIC = (1 << (source - 64));
  } else {
    if (source < 32) *IRQ_DISA1 = (1 << source);
    else if (source < 64) *IRQ_DISA2 = (1 << (source - 32));
    else *IRQ_DISABASIC = (1 << (source - 64));
  }
  callbacks[source] = fn;
}
