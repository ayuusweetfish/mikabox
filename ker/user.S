.global jump_user
jump_user:
  // Client for domain 1, manager for domain 0
  mov   r1, #7
  mcr   p15, 0, r1, c3, c0, 0
  cps   #0x10
  bx    r0
