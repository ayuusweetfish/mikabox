# Uniform 0/1: texture
# Varying 0/1: texture coordinate (0 to 1)
# Varying 2/3/4/5: chroma (RGBA)
# Premultiplied alpha for everything

# Load texture coordinates and issue TMU load
nop; fmul r0, vary, ra15
fadd r0, r0, r5; fmul r1, vary, ra15; lthrsw
fadd t0t, r1, r5; nop
mov t0s, r0; nop

# Pack varyings into r3 (ARGB)
nop; fmul r0, vary, ra15
fadd ra0, r0, r5; fmul r1, vary, ra15
fadd ra1, r1, r5; fmul r0, vary, ra15
fadd ra2, r0, r5; fmul r1, vary, ra15
fadd ra3, r1, r5; mov r3.8c, ra0
nop; mov r3.8b, ra1
nop; mov r3.8a, ra2
nop; mov r3.8d, ra3; ldtmu0

# And multiply with texture sample (ARGB)
nop; v8muld ra0, r3, r4
# ra0 = tinted texture, R/A (R represents any chroma channel hereafter)

nop; nop
fsub ra1.8abcd, 1.0, ra0.8d
nop; nop; loadc
# ra1 = 1-A (packed)
# r4 = canvas, Rcan/Acan

# R' = R + Rcan * (1-A)
# A' = A + Acan * (1-A)

nop; v8muld r2, r4, ra1
v8adds r3, r2, ra0; nop
mov tlbm, r3; nop; loadc

nop; v8muld r2, r4, ra1
v8adds r3, r2, ra0; nop
mov tlbm, r3; nop; loadc

nop; v8muld r2, r4, ra1
v8adds r3, r2, ra0; nop
mov tlbm, r3; nop; loadc

nop; v8muld r2, r4, ra1; thrend
v8adds r3, r2, ra0; nop
mov tlbm, r3; nop; sbdone
