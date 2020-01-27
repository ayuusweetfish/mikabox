#ifndef _Mikabox_priv_h_
#define _Mikabox_priv_h_

#define MODE_USR  0x10
#define MODE_FIQ  0x11
#define MODE_IRQ  0x12
#define MODE_SVC  0x13
#define MODE_ABT  0x17
#define MODE_UND  0x1b
#define MODE_SYS  0x1f

void change_mode(uint32_t mode);
void change_mode_b(uint32_t mode, void *addr);
void set_user_sp(void *sp);
void *get_user_sp();
void jump_user(void *pc);

#include "coroutine.h"

extern struct reg_set user_yield_regs;

#endif
