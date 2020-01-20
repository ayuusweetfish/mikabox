#include "coroutine.h"

static void co_done();

#define MAX_RECURSION 16
static struct coroutine co_toplevel = { .state = CO_STATE_RUN };
struct coroutine *stack[MAX_RECURSION] = { &co_toplevel };
uint16_t stack_top = 0;  // Points directly to the topmost level

__attribute__ ((noinline, naked))
static void co_jump_arg(struct reg_set *save_regs, struct reg_set *load_regs, uint32_t arg)
{
  __asm__ __volatile__ (
    "stmia r0!, {r4-r11, sp, lr}\n"
  );
  // XXX: Can be optimized to one less instruction
  // TODO: This is incorrect, rewrite with assembler
  register uint32_t lr __asm__ ("r0") = (uint32_t)co_done;
  __asm__ __volatile__ (
    "mov lr, %0\n"
    "ldmia r1!, {r4-r11, sp, pc}\n"
    :: "r" (lr)
  );
}

__attribute__ ((noinline, naked))
static void co_jump(struct reg_set *save_regs, struct reg_set *load_regs)
{
  __asm__ __volatile__ (
    "stmia r0!, {r4-r11, sp, lr}\n"
    "ldmia r1!, {r4-r11, sp, pc}\n"
  );
}

void co_create(struct coroutine *co, void (*fn)(uint32_t))
{
  co->state = CO_STATE_NEW;
  co->regs.sp = (uint32_t)&co->stack[0] + CO_STACK;
  co->regs.pc = (uint32_t)fn;
}

void co_start(struct coroutine *co, uint32_t arg)
{
  enum co_state prev_state = co->state;
  co->state = CO_STATE_RUN;
  stack[++stack_top] = co;
  if (prev_state == CO_STATE_NEW)
    co_jump_arg(&stack[stack_top - 1]->regs, &co->regs, arg);
  else
    co_jump(&stack[stack_top - 1]->regs, &co->regs);
}

void co_next(struct coroutine *co)
{
  co->state = CO_STATE_RUN;
  stack[++stack_top] = co;
  co_jump(&stack[stack_top - 1]->regs, &co->regs);
}

void co_yield()
{
  stack_top--;
  stack[stack_top + 1]->state = CO_STATE_YIELD;
  co_jump(&stack[stack_top + 1]->regs, &stack[stack_top]->regs);
}

static void co_done()
{
  stack_top--;
  stack[stack_top + 1]->state = CO_STATE_DONE;
  co_jump(&stack[stack_top + 1]->regs, &stack[stack_top]->regs);
}
