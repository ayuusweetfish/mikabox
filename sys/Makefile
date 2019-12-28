TOOLCHAIN_PREFIX ?= arm-none-eabi-

CC      = $(TOOLCHAIN_PREFIX)gcc
AS      = $(TOOLCHAIN_PREFIX)as
LD      = $(TOOLCHAIN_PREFIX)ld
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy

all:
	$(CC) -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s -nostartfiles -Wl,-T,link.ld -O2 startup.S main.c -o kernel.elf
	$(OBJCOPY) kernel.elf -O binary kernel.img
