TOOLCHAIN_PREFIX ?= arm-none-eabi-

AS      = $(TOOLCHAIN_PREFIX)as
CC      = $(TOOLCHAIN_PREFIX)gcc
LD      = $(TOOLCHAIN_PREFIX)ld
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy

HOSTCC  = gcc

# Verbosity
# Change to empty string for verbose output
V := @

BUILD_DIR := build
C_OPTIONS := -std=c99 -Wall -O2
INCLUDES := -I. -I./ampi/ampi
LIBS := -lm ampi/ampi/libampi.a

SRCS := $(wildcard *.[Sc])

# Library printf
SRCS += printf/printf.c
INCLUDES += -I./printf

OBJS := $(patsubst %.c,%.o,$(SRCS))
OBJS := $(patsubst %.S,%.o,$(OBJS))
OBJS := $(sort $(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_IMG := $(BUILD_DIR)/kernel.img

ARCH_FLAGS := -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s
CFLAGS := $(ARCH_FLAGS) $(C_OPTIONS) $(INCLUDES)
LDFLAGS := $(ARCH_FLAGS) -nostartfiles $(LIBS)

all: $(KERNEL_IMG)  # Default target

EXTRA_OBJS :=
EXTRA_CLEAN :=

$(BUILD_DIR)/charbuf.o: utils/font_bitmap.h
utils/font_bitmap.h: utils/font_bitmap_gen
	@ printf "  %-7s %s\n" RUN utils/font_bitmap_gen
	$(V) cd utils && ./font_bitmap_gen
utils/font_bitmap_gen: utils/font_bitmap_gen.c
	@ printf "  %-7s %-16s %s\n" HOSTCC $<
	$(V) $(HOSTCC) $< -o $@
EXTRA_CLEAN += utils/font_bitmap_gen utils/font_bitmap.h

utils/nanikore.o: utils/nanikore.png
	@ printf "  %-7s %-16s %s\n" FFMPEG $< utils/nanikore.bin
	$(V) ffmpeg -f rawvideo -pix_fmt rgb24 - -i $< > utils/nanikore.bin 2> /dev/null
	@ printf "  %-7s %s\n" LD utils/nanikore.bin
	$(V) $(LD) -b binary utils/nanikore.bin -o $@ 2> /dev/null
EXTRA_OBJS += utils/nanikore.o
EXTRA_CLEAN += utils/nanikore.bin utils/nanikore.o

$(BUILD_DIR)/%.o: BN = $(basename $<)
$(BUILD_DIR)/%.o: %.S %.c
	@ printf "  %-7s %-16s %s\n" AS/CC "$(BN).S/c" "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)/$(dir $<)
	$(V) $(CC) -c $(BN).S -o $(BUILD_DIR)/$(BN).S.o $(CFLAGS)
	$(V) $(CC) -c $(BN).c -o $(BUILD_DIR)/$(BN).c.o $(CFLAGS)
	$(V) $(LD) -r $(BUILD_DIR)/$(BN).S.o $(BUILD_DIR)/$(BN).c.o -o $@

$(BUILD_DIR)/%.o: %.S
	@ printf "  %-7s %-16s %s\n" AS $< "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)/$(dir $<)
	$(V) $(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: %.c
	@ printf "  %-7s %-16s %s\n" CC $< "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)/$(dir $<)
	$(V) $(CC) -c $< -o $@ $(CFLAGS)

$(KERNEL_ELF): $(OBJS) $(EXTRA_OBJS) link.ld
	@ printf "  %-7s %s\n" LD $@
	$(V) $(CC) -Wl,-T,link.ld $(OBJS) $(EXTRA_OBJS) -o $@ $(LDFLAGS)

$(KERNEL_IMG): $(KERNEL_ELF)
	@ printf "  %-7s %-16s %s\n" OBJCOPY $< $@
	$(V) $(OBJCOPY) $< -O binary $@

clean:
	rm -rf $(BUILD_DIR) $(EXTRA_CLEAN)

.PHONY: clean all
