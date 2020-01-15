# Uniform 0/1: texture
# Uniform 2: chroma wetness (0 to 1)
# Varying 0/1: texture coordinate (0 to 1)
# Varying 2/3/4: chroma (RGB)

# Load texture coordinates
nop; fmul r0, vary, ra15
fadd r0, r0, r5; nop
nop; fmul r1, vary, ra15
fadd r1, r1, r5; nop

# Issue TMU read
mov t0t, r1; nop
mov t0s, r0; nop
nop; nop; ldtmu0

# ra0 = k
# rb1 = 1 - k
# val = rb1 + ra0 * val
mov r2, unif; nop
fsub rb1, 1.0, r2; mov ra0, r2

nop; fmul r0, vary, ra15
fadd r0, r0, r5; fmul r1, vary, ra15
fadd r1, r1, r5; fmul r2, vary, ra15
fadd r2, r2, r5; fmul r0, r0, ra0
fadd r0, r0, rb1; fmul r1, r1, ra0
fadd r1, r1, rb1; fmul r2, r2, ra0
fadd r2, r2, rb1; mov r3.8c, r0
nop; mov r3.8b, r1
nop; mov r3.8a, r2; sbwait
nop; mov r3.8d, 1.0

nop; v8muld tlbc, r4, r3; thrend

nop; nop; nop
nop; nop; sbdone
