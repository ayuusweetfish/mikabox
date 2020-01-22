CC := gcc
CMAKE := cmake

# Verbosity
# Change to empty string for verbose output
V := @

BUILD_DIR := build
C_OPTIONS := -std=c99 -Wall -O2
INCLUDES := -I. -I./unicorn/include -I./glew/include -I./glfw/include
STATIC_LIBS := unicorn/libunicorn.a glew/lib/libGLEW.a glfw/build/src/libglfw3.a
LIBS := -lm $(STATIC_LIBS)

SRCS := $(wildcard *.c)

OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(sort $(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))
EMU_EXEC := $(BUILD_DIR)/emu

CFLAGS := $(C_OPTIONS) $(INCLUDES)

all: $(EMU_EXEC)

libs: $(STATIC_LIBS)

unicorn/libunicorn.a:
	@ printf "  %-7s %s\n" MAKE $@
	$(V) $(MAKE) -C $(dir $@) UNICORN_DEBUG=no UNICORN_ARCHS=arm UNICORN_STATIC=yes UNICORN_SHARED=no

glew/lib/libGLEW.a:
	@ printf "  %-7s %s\n" MAKE $@
	$(V) $(MAKE) -C glew glew.lib.static

glfw/build/src/libglfw3.a:
	@ printf "  %-7s %s\n" CMAKE $@
	$(V) mkdir -p glfw/build
	$(V) $(CMAKE) -B glfw/build glfw -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	$(V) $(MAKE) -C glfw/build

$(BUILD_DIR)/swi.o: syscalls.h
$(BUILD_DIR)/syscalls.o: syscalls.h

$(BUILD_DIR)/%.o: %.c
	@ printf "  %-7s %-16s %s\n" CC $(notdir $<) "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)/$(dir $<)
	$(V) $(CC) -c $< -o $@ $(CFLAGS)

$(EMU_EXEC): $(OBJS)
	@ printf "  %-7s %s\n" LD $@
	$(V) $(CC) $^ $(STATIC_LIBS) -o $@ -framework Cocoa -framework OpenGL -framework IOKit

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean all libs $(STATIC_LIBS)
