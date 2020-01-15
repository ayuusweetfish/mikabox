# Uniform 0/1: texture
# Varying 0/1: texture coordinate (0 to 1)
# Varying 2/3/4/5: chroma (RGBA)

# Load texture coordinates
nop; fmul r0, vary, ra15
fadd r0, r0, r5; nop
nop; fmul r1, vary, ra15
fadd r1, r1, r5; nop

# Issue TMU read
mov t0t, r1; nop
mov t0s, r0; nop

# Pack varyings into r3 (ARGB)
nop; fmul r0, vary, ra15; ldtmu0
fadd ra0, r0, r5; fmul r1, vary, ra15
fadd ra1, r1, r5; fmul r0, vary, ra15
fadd ra2, r0, r5; fmul r1, vary, ra15
fadd ra3, r1, r5; mov r3.8c, ra0
nop; mov r3.8b, ra1
nop; mov r3.8a, ra2
nop; mov r3.8d, ra3; sbwait

# And multiply with texture sample (ARGB)
nop; v8muld r0, r3, r4

mov tlbc, r0; thrend
nop; nop
nop; nop; sbdone
