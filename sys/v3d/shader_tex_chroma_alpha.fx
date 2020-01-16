# Uniform 0/1: texture
# Varying 0/1: texture coordinate (0 to 1)
# Varying 2/3/4/5: chroma (RGBA)
# Premultiplied alpha for everything

# Load texture coordinates
nop; fmul r0, vary, ra15
fadd r0, r0, r5; fmul r1, vary, ra15
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
nop; v8muld ra0, r3, r4; loadc
# ra0 = tinted texture, R/A (R represents any chroma channel hereafter)
# r4 = canvas, Rcan/Acan

# R' = R + Rcan * (1-A)
# A' = A + Acan - A*Acan

fadd r0, 0.0, r4.8d; nop
fadd r1, 0.0, ra0.8d; nop
fsub r2, 1.0, r1; mov rb1.8abcd, r1
nop; mov ra2.8abcd, r2
nop; fmul rb3, r0, r1
# r0 = Acan
# r1 = A
# rb1 = A (packed)
# ra2 = 1-A (packed)
# rb3 = A*Acan

fadd r0, r0, r1; v8muld r2, r4, ra2
fsub r0, r0, rb3; v8adds r3, r2, ra0
# r0 (overwrite) = A'
# r2 = Rcan * (1-A) (packed RGB)
# r3 = R' (packed RGB)

mov tlbc, r3; thrend
nop; nop
nop; nop; sbdone
