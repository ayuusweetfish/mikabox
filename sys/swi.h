#ifndef _Mikabox_swi_h_
#define _Mikabox_swi_h_

#include <stdint.h>

uint32_t swi_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);

uint32_t syscall0(uint32_t);
uint32_t syscall1(uint32_t, uint32_t);
uint32_t syscall2(uint32_t, uint32_t, uint32_t);
uint32_t syscall3(uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t syscall4(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define VA_GET(_0, _1, _2, _3, _4, _N, ...) (_N)
#define VA_ARGC(...) VA_GET(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define VA_SYSCALL(_argc, _0, _1, _2, _3, _4, ...) \
  ((_argc) == 0 ? syscall0(_0) : \
   (_argc) == 1 ? syscall1(_1, _0) : \
   (_argc) == 2 ? syscall2(_1, _2, _0) : \
   (_argc) == 3 ? syscall3(_1, _2, _3, _0) : \
   (_argc) == 4 ? syscall4(_1, _2, _3, _4, _0) : 0)
#define syscall(...) \
  VA_SYSCALL(VA_ARGC(__VA_ARGS__) - 1, __VA_ARGS__, 0, 0, 0, 0)

#endif
