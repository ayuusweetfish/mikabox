TOOLCHAIN_PREFIX ?= arm-none-eabi-

AS      = $(TOOLCHAIN_PREFIX)as
CC      = $(TOOLCHAIN_PREFIX)gcc
LD      = $(TOOLCHAIN_PREFIX)ld
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy

# Verbosity
# Change to empty string for verbose output
V := @

BUILD_DIR := build
C_OPTIONS := -std=c99 -O2
INCLUDES := -I.
LIBS := -lm

SRCS := $(wildcard *.[Sc])
OBJS := $(patsubst %.c,%.o,$(SRCS))
OBJS := $(patsubst %.S,%.o,$(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_IMG := $(BUILD_DIR)/kernel.img

CFLAGS := -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s \
  $(C_OPTIONS) $(INCLUDES)
LDFLAGS := -nostartfiles -nostdlib $(LIBS)

$(BUILD_DIR)/%.o: %.S
	@ printf "  %-4s %-16s %s\n" AS $< "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)
	$(V) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@ printf "  %-4s %-16s %s\n" CC $< "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)
	$(V) $(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS) link.ld
	@ printf "  %-4s %s\n" LD $@
	$(V) $(CC) $(LDFLAGS) -nostartfiles -T link.ld $(OBJS) -o $@

$(KERNEL_IMG): $(KERNEL_ELF)
	@ printf "  %-4s %-16s %s\n" COPY $< $@
	$(V) $(OBJCOPY) $< -O binary $@

clean:
	rm -rf $(BUILD_DIR)

.DEFAULT_GOAL := $(KERNEL_IMG)
