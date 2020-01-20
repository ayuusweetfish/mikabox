// Preserves r3
.global change_mode
change_mode:
  mov   r1, lr
  // Fallthrough
.global change_mode_b
change_mode_b:
  mrs   r2, cpsr
  bic   r2, r2, #0x1f
  orr   r2, r2, r0
  msr   cpsr_c, r2
  bx    r1

.global set_user_sp
set_user_sp:
  cps   #0x1f
  mov   sp, r0
  cps   #0x13
  bx    lr

.global get_user_sp
get_user_sp:
  cps   #0x1f
  mov   r0, sp
  cps   #0x13
  bx    lr
