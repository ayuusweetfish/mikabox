#ifndef _Mikabox_coroutine_h_
#define _Mikabox_coroutine_h_

#ifdef __cplusplus
extern "C" {
#endif

// Implementation of a simple co-operative scheduler.

// co_next() may only be called on the main thread and
// executes a thread until it calls co_yield().

// If co_yield() is called on the main thread, co_next()
// will be called for each active thread instead.

#include <stdint.h>

#define CO_STACK  65536

struct reg_set {
  uint64_t d[16];
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t sp;
  uint32_t pc;
} __attribute__((packed, aligned(8)));

struct coroutine {
  struct reg_set regs;
  enum co_state {
    CO_STATE_NEW = 0,
    CO_STATE_RUN,
    CO_STATE_YIELD,
    CO_STATE_DONE
  } state;
  uint8_t stack[CO_STACK] __attribute__((aligned(8)));
} __attribute__((aligned(8)));

void co_create(struct coroutine *co, void (*fn)(uint32_t));
void co_start(struct coroutine *co, uint32_t arg);
void co_next(struct coroutine *co);
void co_yield();

#ifdef __cplusplus
}
#endif

#endif
