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

#define MAX_CO  8

int8_t co_create(void (*fn)(void *), void *arg);
void co_yield();
void co_next(int8_t id);

#ifdef __cplusplus
}
#endif

#endif
