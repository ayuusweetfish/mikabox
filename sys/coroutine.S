.global co_jump_arg
co_jump_arg:
  fstmiad r0!, {d0-d15}
  stmia r0!, {r4-r11, sp, lr}
  mov   r0, r2
  ldr   lr, =co_done
  fldmiad r1!, {d0-d15}
  ldmia r1!, {r4-r11, sp, pc}

.global co_jump
co_jump:
  fstmiad r0!, {d0-d15}
  stmia r0!, {r4-r11, sp, lr}
  fldmiad r1!, {d0-d15}
  ldmia r1!, {r4-r11, sp, pc}
