.global co_jump_arg
co_jump_arg:
  stmia r0!, {r4-r11, sp, lr}
  mov r0, r2
  ldr lr, =co_done
  ldmia r1!, {r4-r11, sp, pc}

.global co_jump
co_jump:
  stmia r0!, {r4-r11, sp, lr}
  ldmia r1!, {r4-r11, sp, pc}
