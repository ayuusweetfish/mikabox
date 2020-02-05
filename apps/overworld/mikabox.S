.global syscall0
syscall0:
  push  {r7, lr}
  mov   r7, r0
  swi   #0
  pop   {r7, pc}

.global syscall1
syscall1:
  push  {r7, lr}
  mov   r7, r1
  swi   #0
  pop   {r7, pc}

.global syscall2
syscall2:
  push  {r7, lr}
  mov   r7, r2
  swi   #0
  pop   {r7, pc}

.global syscall3
syscall3:
  push  {r7, lr}
  mov   r7, r3
  swi   #0
  pop   {r7, pc}

.global syscall4
syscall4:
  push  {r7, lr}
  ldr   r7, [sp, #8]
  swi   #0
  pop   {r7, pc}

.global _sbrk
_sbrk:
  mov   r0, #0
  ldr   r1, =_unimplemented
  mov   r2, #7
  b     syscall2

_unimplemented: .ascii  "_sbrk() unimplemented"
