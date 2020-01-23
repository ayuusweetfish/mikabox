#define SYSCALLS_IMPL 1
#include "syscalls.h"

#include "unicorn/unicorn.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

extern uint64_t syscalls_lcg;

void syscall_reinit_rng()
{
  FILE *fp_random;
  if ((fp_random = fopen("/dev/urandom", "r")) == NULL ||
    fread(&syscalls_lcg, 8, 1, fp_random) < 1
  ) {
    printf("fopen(\"/dev/urandom\") returned error %d (%s)\n",
      errno, strerror(errno));
    printf("Random numbers will be based on current time.\n");
    syscalls_lcg = time(NULL);
  }
  if (fp_random) fclose(fp_random);
  printf("Random number generator reinitialized\n");
}

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
  syscall_reinit_rng();
}

extern uc_engine *syscalls_uc;

void syscall_read_mem(uint32_t addr, uint32_t size, void *buf)
{
  uc_err err;
  if ((err = uc_mem_read(syscalls_uc, addr, buf, size)) != UC_ERR_OK) {
    printf("uc_mem_read() returned error %u (%s)\n", err, uc_strerror(err));
    exit(1);
  }
}
