TOOLCHAIN_PREFIX ?= arm-none-eabi-
CC = $(TOOLCHAIN_PREFIX)gcc

# Verbosity
# Change to empty string for verbose output
V := @

BUILD_DIR := build
ARCH_FLAGS := -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s

INCLUDES := -I./wren/src/include
DEFINES := -DWREN_OPT_META=0 -DWREN_OPT_RANDOM=0
SRCS := $(wildcard **.c) $(wildcard wren/src/vm/**.c)

CFLAGS := $(ARCH_FLAGS) -std=c99 -Wall -Wno-main -O2 $(DEFINES) $(INCLUDES) -fno-builtin-printf
LDFLAGS := $(ARCH_FLAGS) -nostartfiles -lm -Wl,-z,max-page-size=4096 -Wl,--gc-sections

OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(patsubst %.S, %.o, $(OBJS))
OBJS := $(sort $(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))

APP := $(BUILD_DIR)/a.out

all: $(APP)

$(BUILD_DIR)/%.o: %.c
	@ printf "  %-7s %-16s %s\n" CC $(notdir $<) "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)/$(dir $<)
	$(V) $(CC) -c $< -o $@ $(CFLAGS)

$(APP): $(OBJS) link.ld
	@ printf "  %-7s %s\n" LD $@
	$(V) $(CC) -Wl,-T,link.ld $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

run: $(APP)
	../../emu/build/emu $< .

.PHONY: all clean run
