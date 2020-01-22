#ifndef _Mikabox_swi_h_
#define _Mikabox_swi_h_

#include "unicorn/unicorn.h"
#include <stdint.h>

void handler_syscall(uc_engine *uc, uint32_t intno, void *user_data);

#endif
