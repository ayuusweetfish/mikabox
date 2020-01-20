.section ".text.startup"

.global startup
startup:
  // Interrupt vector table
  // Will branch to _reset initially due to the first entry
  ldr   pc, _addr_reset
  ldr   pc, _addr_int_uinstr
  ldr   pc, _addr_int_swi
  ldr   pc, _addr_int_pfabort
  ldr   pc, _addr_int_dabort
  ldr   pc, _addr_int_uhandler
  ldr   pc, _addr_int_irq
  ldr   pc, _addr_int_fiq

_addr_reset:        .word _reset
_addr_int_uinstr:   .word _reset
_addr_int_swi:      .word _swi_stub
_addr_int_pfabort:  .word _reset
_addr_int_dabort:   .word _reset
_addr_int_uhandler: .word _reset
_addr_int_irq:      .word _irq_stub
_addr_int_fiq:      .word _reset

_swi_stub:
  stmfd sp!, {ip, lr}
  cmp   r7, #1      // GEN + 1, yield
  beq   _swi_stub_cont

  // Save registers
  ldr   r0, =user_yield_regs  // External
  ldr   r1, =_swi_stub_cont
  b     regs_save             // External

_swi_stub_cont:
  bl    swi_handler // External symbol
  ldmfd sp!, {ip, pc}^

_irq_stub:
  mov   sp, #0x4000
  sub   lr, lr, #4  // Return address
  stmfd sp!, {r0-r12, lr}
  mov   r0, lr
  bl    irq_handler // External symbol
  ldmfd sp!, {r0-r12, pc}^

_reset:
  // Initialize interrupt vector table
  // Copy instructions and constant values to address 0x0
  mov   r0, #0x8000
  mov   r1, #0x0
  ldmia r0!, {r2-r9}
  stmia r1!, {r2-r9}
  ldmia r0!, {r2-r9}
  stmia r1!, {r2-r9}

  // Initialize stack pointer
  mov   sp, #0x8000

  // Enable FPU
  mrc   p15, 0, r0, c1, c0, 2
  orr   r0, r0, #0xf00000 // Single & double precision
  mcr   p15, 0, r0, c1, c0, 2
  mov   r0, #0x40000000   // Set EN bit
  fmxr  fpexc, r0

  // Enable IRQ
  cpsie i

  bl    sys_main

_loop:
  // In case kernel returns O_O
  b     _loop
