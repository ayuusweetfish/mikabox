#ifndef _Mikabox_swi_h_
#define _Mikabox_swi_h_

#include <stdint.h>

uint32_t swi_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);

uint32_t syscall0(uint32_t num);
uint32_t syscall1(uint32_t num, uint32_t);
uint32_t syscall2(uint32_t num, uint32_t, uint32_t);
uint32_t syscall3(uint32_t num, uint32_t, uint32_t, uint32_t);
uint32_t syscall4(uint32_t num, uint32_t, uint32_t, uint32_t, uint32_t);

#endif
