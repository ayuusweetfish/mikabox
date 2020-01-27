# Varying 0/1/2/3: chroma (RGBA)

nop; fmul r0, vary, ra15
fadd ra0, r0, r5; fmul r1, vary, ra15
fadd ra1, r1, r5; fmul r0, vary, ra15
fadd ra2, r0, r5; fmul r1, vary, ra15
fadd r0, r1, r5; mov r3.8c, ra0
nop; mov r3.8b, ra1
nop; mov r3.8a, ra2
nop; mov r3.8d, r0
# r0 = A
# r3 = chroma (packed)

fsub r1, 1.0, r0; mov ra0, r3
nop; mov r3.8abcd, r1; loadc
# ra0 = chroma (packed)
# r3 = 1-A (packed)
# r4 = canvas, Rcan/Acan

nop; v8muld r2, r4, r3; loadc
v8adds tlbm, r2, ra0; nop

nop; v8muld r2, r4, r3; loadc
v8adds tlbm, r2, ra0; nop

nop; v8muld r2, r4, r3; loadc
v8adds tlbm, r2, ra0; nop; thrend

nop; v8muld r2, r4, r3
v8adds tlbm, r2, ra0; nop; sbdone
