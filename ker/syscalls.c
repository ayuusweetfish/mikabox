#include "printf/printf.h"

#define SYSCALLS_IMPL 1
#include "syscalls.h"

static inline void assert_log(
  const char *file, uint32_t line,
  const char *func, const char *cond)
{
  printf("Assertion failed: %s\n"
    "  at %s (%s:%d)\n",
    cond, func, file, line);
  while (1) { }
}

#define assert(__cond) do { \
  if (!(__cond)) assert_log(__FILE__, __LINE__, __func__, #__cond); \
} while (0)

void syscalls_init()
{
  #define SYSCALLS_INIT 1
  #include "syscalls.h"
}
