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
  // Change to System mode
  mrs   r2, cpsr
  orr   r2, r2, #0x1f
  msr   cpsr_c, r2
  // Set SP
  mov   sp, r0
  // Change back to Supervisor mode
  bic   r2, r2, #0xc
  msr   cpsr_c, r2
  bx    lr
