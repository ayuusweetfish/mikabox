#define SYSCALLS_IMPL 1
#include "syscalls.h"

#undef SYSCALLS_IMPL
#define SYSCALLS_INIT 1

#include "unicorn/unicorn.h"
#include <stdio.h>

void syscalls_init()
{
  #include "syscalls.h"
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
