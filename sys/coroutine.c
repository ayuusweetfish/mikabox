#include "coroutine.h"

#define MAX_STACK   65536

static uint32_t used = 0;
static void (*fn_ptr[MAX_CO])(void *);
static void *args[MAX_CO];
static int8_t stack_space[MAX_CO][MAX_STACK];

static int8_t current = 0;

struct reg_set {
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
};

static struct reg_set main_regs;
static struct reg_set regs[MAX_CO] = {{ 0 }};

static void co_done();

__attribute__ ((noinline, naked))
void co_jump_arg(struct reg_set *restore_regs, struct reg_set *branch_regs, void *arg)
{
  uint32_t lr = (uint32_t)co_done;
  __asm__ __volatile__ (
    "stmia r0!, {r4-r11, sp, lr}\n"
    "mov r0, r2\n"
    "mov lr, %0\n"
    "ldmia r1!, {r4-r11, sp, pc}\n"
    :: "r"(lr)
  );
}

__attribute__ ((noinline, naked))
void co_jump(struct reg_set *restore_regs, struct reg_set *branch_regs)
{
  __asm__ __volatile__ (
    "stmia r0!, {r4-r11, sp, lr}\n"
    "ldmia r1!, {r4-r11, sp, pc}\n"
  );
}

int8_t co_create(void (*fn)(void *), void *arg)
{
  for (int i = 0; i < MAX_CO; i++)
    if (!(used & (1 << i))) {
      used |= (1 << i);
      fn_ptr[i] = fn;
      args[i] = arg;
      return i + 1;
    }
  return 0;
}

void co_yield()
{
  if (current > 0) {
    int8_t id = current - 1;
    current = 0;
    co_jump(&regs[id], &main_regs);
  }
}

static void co_done()
{
  if (current > 0) {
    int8_t id = current - 1;
    used &= ~(1 << id);
    regs[id].pc = 0;
    current = 0;
    co_jump(&regs[id], &main_regs);
  }
}

void co_next(int8_t id)
{
  id--;
  if (!(used & (1 << id))) return;
  current = id + 1;
  if (regs[id].pc == 0) {
    // Initialization
    regs[id].pc = (uint32_t)fn_ptr[id];
    regs[id].sp = (uint32_t)&stack_space[id + 1];
    co_jump_arg(&main_regs, &regs[id], args[id]);
  } else {
    co_jump(&main_regs, &regs[id]);
  }
}