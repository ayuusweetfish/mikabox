# Varying 0/1/2: chroma (RGB)

nop; fmul r0, vary, ra15
fadd ra0, r0, r5; fmul r1, vary, ra15
fadd ra1, r1, r5; fmul r0, vary, ra15
fadd ra2, r0, r5; mov r3.8d, 1.0
nop; mov r3.8c, ra0
nop; mov r3.8b, ra1
nop; mov r3.8a, ra2; sbwait
mov tlbc, r3; nop; thrend
nop; nop
nop; nop; sbdone
