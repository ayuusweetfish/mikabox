.global jump_user
jump_user:
  // Client for domain 1, manager for domain 0
  mov   r1, #5
  mcr   p15, 0, r1, c3, c0, 0
  cps   #0x10
  bx    r0

.global userovo
userovo:
  mov   r7, #43
  ldr   r0, =#555
  swi   0
here:
  b     here
