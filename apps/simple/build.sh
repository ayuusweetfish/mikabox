#!/bin/sh
arm-none-eabi-gcc -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s -nostartfiles -Wl,-T,link.ld -Wl,-z,max-page-size=4096 -std=c99 -O2 ${1:-"main.c"} mikabox.S -lm -o a.out
