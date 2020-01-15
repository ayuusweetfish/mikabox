# Varying 0/1/2/3: chroma (RGBA)

nop; fmul r0, vary, ra15
fadd ra0, r0, r5; fmul r1, vary, ra15
fadd ra1, r1, r5; fmul r0, vary, ra15
fadd ra2, r0, r5; fmul r1, vary, ra15
fadd r0, r1, r5; nop
# ra0 = R
# ra1 = G
# ra2 = B
# r0 = A

fsub r1, 1.0, r0; fmul ra0, ra0, r0
nop; fmul ra1, ra1, r0; sbwait
nop; fmul ra2, ra2, r0; loadc
# ra0-2 = alpha multiplied
# r1 = 1 - A
# r4 = canvas

# A' = 1 - (1-A) * (1-Acan) = A + Acan - A*Acan
# R' = Rcan * (1-A) + R * A
fadd ra3, ra0, r4.8d; fmul rb4, ra0, r4.8d
# ra3 = A + Acan
# rb4 = A * Acan
fadd rb16, ra3, rb4; fmul r2, r4.8c, r1
# rb16 = A'
# r2 = Rcan * (1-A)
fadd rb17, r2, ra0; fmul r0, r4.8b, r1
fadd rb18, r0, ra1; fmul r2, r4.8a, r1
fadd rb19, r2, ra2; mov r3.8d, rb16
# r0 scratched
# rb17-19 = R', G', B'
nop; mov r3.8c, rb17
nop; mov r3.8b, rb18
nop; mov r3.8a, rb19

mov tlbc, r3; thrend
nop; nop
nop; nop; sbdone
