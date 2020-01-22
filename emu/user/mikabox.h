#ifndef _Mikabox_mikabox_h_
#define _Mikabox_mikabox_h_

#include <stdint.h>

uint64_t syscall0(uint32_t);
uint64_t syscall1(uint32_t, uint32_t);
uint64_t syscall2(uint32_t, uint32_t, uint32_t);
uint64_t syscall3(uint32_t, uint32_t, uint32_t, uint32_t);
uint64_t syscall4(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define VA_GET(_0, _1, _2, _3, _4, _N, ...) (_N)
#define VA_ARGC(...) VA_GET(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define VA_SYSCALL(_argc, _0, _1, _2, _3, _4, ...) \
  ((_argc) == 0 ? syscall0((uint32_t)(_0)) : \
   (_argc) == 1 ? syscall1((uint32_t)(_1), (uint32_t)(_0)) : \
   (_argc) == 2 ? syscall2((uint32_t)(_1), (uint32_t)(_2), (uint32_t)(_0)) : \
   (_argc) == 3 ? syscall3((uint32_t)(_1), (uint32_t)(_2), (uint32_t)(_3), (uint32_t)(_0)) : \
   (_argc) == 4 ? syscall4((uint32_t)(_1), (uint32_t)(_2), (uint32_t)(_3), (uint32_t)(_4), (uint32_t)(_0)) : 0)
#define syscall64(...) \
  VA_SYSCALL(VA_ARGC(__VA_ARGS__) - 1, __VA_ARGS__, 0, 0, 0, 0)
#define syscall(...) ((uint32_t)syscall64(__VA_ARGS__))
#pragma GCC diagnostic ignored "-Wunused-value"

#endif
