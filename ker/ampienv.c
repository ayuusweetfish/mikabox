#include "ampienv.h"
#include "common.h"
#include "kmalloc.h"
#include "irq.h"
#include "prop_tag.h"
#include "printf/printf.h"
#include "coroutine.h"

void *ampi_malloc (size_t size)
{
  return kmalloc(size);
}

void ampi_free (void *ptr)
{
  kfree(ptr);
}

void ampi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
}

void MsDelay (unsigned nMilliSeconds)
{
  usDelay(nMilliSeconds * 1000);
}

void usDelay (unsigned nMicroSeconds)
{
  mem_barrier();
  uint32_t val = *TMR_CLO + nMicroSeconds;
  while (*TMR_CLO < val) {
    if (nMicroSeconds > 5000) co_yield();
    mem_barrier();
  }
  mem_barrier();
}

extern void (*periodic)();

void RegisterPeriodicHandler (TPeriodicTimerHandler *pHandler)
{
  periodic = pHandler;
}

void ConnectInterrupt (unsigned nIRQ, TInterruptHandler *pHandler, void *pParam)
{
  irq_set_callback(nIRQ, pHandler, pParam);
}

uint32_t EnableVCHIQ (uint32_t p)
{
  return enable_vchiq(p);
}

uint32_t y = 0;

void LogWrite (const char *pSource,
               unsigned    Severity,
               const char *pMessage, ...)
{
  if (Severity <= LOG_ERROR) {y++;return;}
  printf("[%c] %s: ", "!EWND"[Severity], pSource ? pSource : "undef");
  va_list arglist;
  va_start(arglist, pMessage);
  vprintf(pMessage, arglist);
  va_end(arglist);
  _putchar('\n');
}

uint8_t coh[512 * 1024]
  __attribute__((section(".bss.ord"), aligned(4096)));

void *GetCoherentRegion512K ()
{
  return coh;
}
