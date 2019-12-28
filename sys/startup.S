.section ".text.startup"

.global startup
startup:
  // Initialize stack pointer
  mov   sp, #0x8000

  // Enable FPU
  mrc   p15, 0, r0, c1, c0, 2
  orr   r0, r0, #0xf00000 // Single & double precision
  mcr   p15, 0, r0, c1, c0, 2
  mov   r0, #0x40000000   // Set EN bit
  fmxr  fpexc, r0

  b     sys_main

_loop:
  // In case kernel returns O_O
  b     _loop
