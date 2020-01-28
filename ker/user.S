.global jump_user
jump_user:
  // Client for domain 1, manager for domain 0
  mov   r1, #7
  mcr   p15, 0, r1, c3, c0, 0
  cps   #0x10
  bx    r0

.global userovo
userovo:
  ldr   r4, =#555
userovovo:
  mov   r7, #42
  mov   r0, r4
  swi   0
  mov   r7, #1
  swi   0

  ldr   r0, =#3000555
  cmp   r4, r0
  beq   vsync_callback

  add   r4, #1
  b     userovovo
