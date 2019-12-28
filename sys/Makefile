TOOLCHAIN_PREFIX ?= arm-none-eabi-

AS      = $(TOOLCHAIN_PREFIX)as
CC      = $(TOOLCHAIN_PREFIX)gcc
LD      = $(TOOLCHAIN_PREFIX)ld
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy

# Verbosity
# Change to empty string for verbose output
V := @

BUILD_DIR := build
C_OPTIONS := -O2

S_SRCS := $(wildcard *.S)
C_SRCS := $(wildcard *.c)
S_OBJS := $(addprefix $(BUILD_DIR)/, $(notdir $(S_SRCS:.S=.o)))
C_OBJS := $(addprefix $(BUILD_DIR)/, $(notdir $(C_SRCS:.c=.o)))
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_IMG := $(BUILD_DIR)/kernel.img

CFLAGS := -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s $(C_OPTIONS)

$(BUILD_DIR)/%.o: %.c
	@ printf "  %-4s %-16s %s\n" CC $< "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)
	$(V) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@ printf "  %-4s %-16s %s\n" CC $< "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)
	$(V) $(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(S_OBJS) $(C_OBJS) link.ld
	@ printf "  %-4s %s\n" LD $@
	$(V) $(LD) $(LDFLAGS) -nostartfiles -T link.ld $(S_OBJS) $(C_OBJS) -o $@

$(KERNEL_IMG): $(KERNEL_ELF)
	@ printf "  %-4s %-16s %s\n" COPY $< $@
	$(V) $(OBJCOPY) $< -O binary $@

clean:
	rm -rf $(BUILD_DIR)

all: $(KERNEL_IMG)
