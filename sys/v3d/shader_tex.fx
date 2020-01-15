# Uniform 0/1: texture
# Varying 0/1: texture coordinate (0 to 1)

# Load texture coordinates
nop; fmul r0, vary, ra15
fadd r0, r0, r5; nop
nop; fmul r1, vary, ra15
fadd r1, r1, r5; nop

# Issue TMU read
mov t0t, r1; nop
mov t0s, r0; nop
nop; nop; ldtmu0
nop; nop; sbwait

mov tlbc, r4; nop; thrend

nop; nop
nop; nop; sbdone
