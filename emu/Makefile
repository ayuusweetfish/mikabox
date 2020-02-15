CC := gcc
CMAKE := cmake

# Verbosity
# Change to empty string for verbose output
V := @

ifdef DEBUG
OPT_FLAGS := -g -Og
else
OPT_FLAGS := -O2
endif

BUILD_DIR := build
C_OPTIONS := -std=c99 -Wall $(OPT_FLAGS)
INCLUDES := -I. -I./common -I./unicorn/include -I./glew/include -I./glfw/include
STATIC_LIBS := unicorn/libunicorn.a glew/lib/libGLEW.a glfw/build/src/libglfw3.a
STATIC_LIB_TARGETS := $(patsubst %.a, %.a.target, $(STATIC_LIBS))
LIBS := -lm $(STATIC_LIBS)

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
  C_OPTIONS += -D_DARWIN_C_SOURCE
  LDFLAGS := -framework Cocoa -framework OpenGL -framework IOKit
endif
ifeq ($(UNAME), Linux)
  C_OPTIONS += -D_XOPEN_SOURCE=700 -D_BSD_SOURCE -D_GNU_SOURCE
  LDFLAGS := -lGL -lX11 -lpthread -lm -ldl
endif

SRCS := $(wildcard *.c) $(wildcard common/*.c)
SRCS += timer_lib/timer.c

OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(sort $(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))
EMU_EXEC := $(BUILD_DIR)/emu

CFLAGS := $(C_OPTIONS) $(INCLUDES)

all: $(EMU_EXEC)

libs: $(STATIC_LIB_TARGETS)

unicorn/libunicorn.a.target:
	@ printf "  %-7s %s\n" MAKE $@
	$(V) cd unicorn && git apply --check ../unicorn.diff && git apply ../unicorn.diff || true
	$(V) $(MAKE) -C $(dir $@) UNICORN_DEBUG=no UNICORN_ARCHS=arm UNICORN_STATIC=yes UNICORN_SHARED=no

glew/lib/libGLEW.a.target:
	@ printf "  %-7s %s\n" MAKE $@
	$(V) $(MAKE) -C glew glew.lib.static

glfw/build/src/libglfw3.a.target:
	@ printf "  %-7s %s\n" CMAKE $@
	$(V) mkdir -p glfw/build
	$(V) cd glfw/build && $(CMAKE) .. -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	$(V) $(MAKE) -C glfw/build

$(BUILD_DIR)/swi.o: syscalls.h
$(BUILD_DIR)/syscalls.o: syscalls.h

$(BUILD_DIR)/%.o: FLATDIR = $(subst ./, , $(firstword $(subst /, , $(dir $<)))/)
$(BUILD_DIR)/%.o: %.c
	@ printf "  %-7s %-16s %s\n" CC $(FLATDIR)$(notdir $<) "$(C_OPTIONS)"
	$(V) mkdir -p $(BUILD_DIR)/$(dir $<)
	$(V) $(CC) -c $< -o $@ $(CFLAGS)

$(EMU_EXEC): $(OBJS) $(STATIC_LIBS)
	@ printf "  %-7s %s\n" LD $@
	$(V) $(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

clean_libs:
	cd unicorn && make clean && git checkout -- .
	cd glew && make clean
	rm -rf glfw/build

.PHONY: clean all libs $(STATIC_LIB_TARGETS)
