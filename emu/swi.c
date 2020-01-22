#include "swi.h"
#include "syscalls.h"
#include <stddef.h>

typedef uint64_t (*syscall_fn_t)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);

static const syscall_fn_t fn_table[4096] = {
  #define SYSCALLS_TABLE 1
  #include "syscalls.h"
};

uc_engine *syscalls_uc;

void handler_syscall(uc_engine *uc, uint32_t intno, void *user_data)
{
  syscalls_uc = uc;
  uc_err err;

  static const int regids[] = {
    UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_R7
  };
  uint32_t r0, r1, r2, r3, r7;
  void *p[] = {&r0, &r1, &r2, &r3, &r7};
  if ((err = uc_reg_read_batch(uc, (int *)regids, p, 5)) != UC_ERR_OK) {
    printf("uc_reg_read_batch() returned error %u (%s)\n", err, uc_strerror(err));
    exit(1);
  }

  uint32_t num = r7;
  syscall_fn_t fn = fn_table[num & 4095];

  uint64_t retval = 0;
  if (fn != NULL) retval = (*fn)(r0, r1, r2, r3);

  r0 = (retval & 0xffffffff);
  r1 = (retval >> 32);

  if ((err = uc_reg_write_batch(uc, (int *)regids, p, 2)) != UC_ERR_OK) {
    printf("uc_reg_write_batch() returned error %u (%s)\n", err, uc_strerror(err));
    exit(1);
  }
}
