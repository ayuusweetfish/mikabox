# Uniform 0/1: texture
# Varying 0/1: texture coordinate (0 to 1)

# Load texture coordinates and issue TMU load
nop; fmul r0, vary, ra15
fadd r0, r0, r5; fmul r1, vary, ra15
fadd t0t, r1, r5; nop
mov t0s, r0; nop; sbwait

nop; nop; ldtmu0
mov tlbc, r4; nop; thrend

nop; nop
nop; nop; sbdone
