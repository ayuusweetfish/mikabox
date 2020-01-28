#include "uspios.h"
#include "regs.h"
#include "kmalloc.h"
#include "prop_tag.h"
#include "printf/printf.h"

void *malloc(size_t size)
{
  return kmalloc(size);
}

void free(void *ptr)
{
  kfree(ptr);
}

void uspi_assertion_failed(const char *pExpr, const char *pFile, unsigned nLine)
{
  // Dummy
}

extern uint32_t uspi_tick;
#define NUM_TIMERS  32
struct timer {
  uint32_t tick;
  TKernelTimerHandler *callback;
  void *arg;
  void *context;
} timers[NUM_TIMERS] = {{ 0 }};

void uspi_upd_timers()
{
  for (uint16_t i = 0; i < NUM_TIMERS; i++)
    if (timers[i].callback != NULL &&
      (int32_t)(timers[i].tick - uspi_tick) <= 0)
    {
      (*timers[i].callback)(i + 1, timers[i].arg, timers[i].context);
      timers[i].callback = NULL;
    }
}

unsigned StartKernelTimer(
  unsigned nHzDelay, TKernelTimerHandler *pHandler,
  void *pParam, void *pContext)
{
  for (uint16_t i = 0; i < NUM_TIMERS; i++)
    if (timers[i].callback == NULL) {
      timers[i].tick = uspi_tick + nHzDelay;
      timers[i].callback = pHandler;
      timers[i].arg = pParam;
      timers[i].context = pContext;
      return i + 1;
    }
  printf("! timers full\n");
  return 0;
}

void CancelKernelTimer(unsigned hTimer)
{
  if (hTimer >= 1 && hTimer <= NUM_TIMERS)
    timers[hTimer - 1].callback = NULL;
}

void CancelAllKernelTimers()
{
  for (uint16_t i = 0; i < NUM_TIMERS; i++)
    timers[i].callback = NULL;
}

int SetPowerStateOn(unsigned nDeviceId)
{
  bool succeeded = set_power_state(nDeviceId, 3); // on | wait
  return succeeded;
}

int GetMACAddress(unsigned char Buffer[6])
{
  get_mac_addr(Buffer);
  return 1;
}

void DebugHexdump(const void *pBuffer, unsigned nBufLen, const char *pSource)
{
  printf("%s: hex dump of %u byte%s\n",
    pSource, nBufLen, nBufLen == 1 ? "" : "s");
  for (size_t i = 0; i < nBufLen; i += 16) {
    printf("%p: ", pBuffer + i);
    for (size_t j = 0; j < 16; j++)
      printf("%02x%c", *(uint8_t *)(pBuffer + i + j),
        j == 7 ? '-' : (j == 15 ? '\n' : ' '));
  }
}
