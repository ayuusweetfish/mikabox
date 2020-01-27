#include "common.h"

void send_mail(uint32_t data, uint8_t channel)
{
  mem_barrier();
  while ((*MAIL0_STATUS) & (1u << 31)) { }
  *MAIL0_WRITE = (data << 4) | (channel & 15);
  mem_barrier();
}

uint32_t recv_mail(uint8_t channel)
{
  mem_barrier();
  do {
    while ((*MAIL0_STATUS) & (1u << 30)) { }
    uint32_t data = *MAIL0_READ;
    if ((data & 15) == channel) {
      mem_barrier();
      return (data >> 4);
    }
  } while (1);
}
