#include "emu.h"
#include "elf.h"
#include "swi.h"
#include "syscalls.h"
#include "v3d_wrapper.h"
#include "audio_wrapper.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "miniaudio/miniaudio.h"
#include "timer_lib/timer.h"
#include "unicorn/unicorn.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define MEM_START_O 0x40000000
#define MEM_SIZE_O  0x01000000  // 16 MiB
#define MEM_END_O   (MEM_START_O + MEM_SIZE_O)
#define MEM_START_U 0x80000000
#define MEM_SIZE_U  0x18000000  // 384 MiB
#define MEM_END_U   (MEM_START_U + MEM_SIZE_U)

#define PAGE_SIZE   0x100000
#define STACK_SIZE  0x100000    // 1 MiB

#define WIN_W 800
#define WIN_H 480

int8_t routine_id;
uint32_t routine_pc[8];
uc_context *routine_ctx[8];

uint64_t app_tick;
uint32_t req_flags;

// GLFW

static GLFWwindow *window; 

static void glfw_err_callback(int error, const char *desc)
{
  printf("GLFW errors with code %d (%s)\n", error, desc);
}

static void glfw_fbsz_callback(GLFWwindow *window, int w, int h)
{
  glViewport(0, 0, w, h);
}

void setup_glfw()
{
  glfwSetErrorCallback(glfw_err_callback);

  if (!glfwInit()) {
    printf("Cannot initialize GLFW\n");
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  window = glfwCreateWindow(WIN_W, WIN_H, "Mikabox Emulator", NULL, NULL);
  if (window == NULL) {
    printf("Cannot create GLFW window\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    printf("Cannot initialize GLEW\n");
    exit(1);
  }

  glfwSetFramebufferSizeCallback(window, glfw_fbsz_callback);

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

// Audio

static void setup_audio()
{
  ma_device_config dev_config =
    ma_device_config_init(ma_device_type_playback);
  dev_config.playback.format = ma_format_s16;
  dev_config.playback.channels = 2;
  dev_config.sampleRate = 44100;
  dev_config.dataCallback = (ma_device_callback_proc)audio_callback;

  if (ma_device_init(NULL, &dev_config, &audio_device) != MA_SUCCESS ||
      ma_device_start(&audio_device) != MA_SUCCESS) {
    printf("Cannot start audio playback\n");
    exit(1);
  }
}

// Time and events
static tick_t start_time;
static double usec_rate;

int num_players;
uint64_t player_btns[MAX_PLAYERS];
uint64_t player_axes[MAX_PLAYERS];

void initialize_tick()
{
  timer_lib_initialize();
  start_time = timer_current();
  app_tick = 0;

  tick_t tps = timer_ticks_per_second();
  usec_rate = 1e6 / tps;
}

void update_tick()
{
  tick_t diff = timer_current() - start_time;
  app_tick = (tick_t)(diff * usec_rate);
}

static void update_input()
{
  // TODO: Multiple gamepads
  num_players = 1;
  player_btns[0] =
    (glfwGetKey(window, GLFW_KEY_UP) << 0) |
    (glfwGetKey(window, GLFW_KEY_DOWN) << 1) |
    (glfwGetKey(window, GLFW_KEY_LEFT) << 2) |
    (glfwGetKey(window, GLFW_KEY_RIGHT) << 3) |
    (glfwGetKey(window, GLFW_KEY_C) << 4) |
    (glfwGetKey(window, GLFW_KEY_X) << 5) |
    (glfwGetKey(window, GLFW_KEY_Z) << 6) |
    (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) << 7) |
    (glfwGetKey(window, GLFW_KEY_W) << 0) |     // Alternative set of keys
    (glfwGetKey(window, GLFW_KEY_S) << 1) |
    (glfwGetKey(window, GLFW_KEY_A) << 2) |
    (glfwGetKey(window, GLFW_KEY_D) << 3) |
    (glfwGetKey(window, GLFW_KEY_K) << 4) |
    (glfwGetKey(window, GLFW_KEY_L) << 5) |
    (glfwGetKey(window, GLFW_KEY_J) << 6) |
    (glfwGetKey(window, GLFW_KEY_I) << 7);
  if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
      player_btns[0] |=
        (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] << 0) |
        (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] << 1) |
        (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] << 2) |
        (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] << 3) |
        (state.buttons[GLFW_GAMEPAD_BUTTON_A] << 4) |
        (state.buttons[GLFW_GAMEPAD_BUTTON_B] << 5) |
        (state.buttons[GLFW_GAMEPAD_BUTTON_X] << 6) |
        (state.buttons[GLFW_GAMEPAD_BUTTON_Y] << 7);
  }
}

// Unicorn Engine

static uc_engine *uc;

static inline uint32_t align(uint32_t addr, uint32_t align)
{
  return (addr + align - 1) & ~(align - 1);
}

static void *alloc(elf_word vaddr, elf_word memsz, elf_word flags)
{
  void *p = malloc(memsz);

  uint32_t perms = 0;
  if (flags & 4) perms |= UC_PROT_READ;
  if (flags & 2) perms |= UC_PROT_WRITE;
  if (flags & 1) perms |= UC_PROT_EXEC;

  uc_err err;
  if ((err = uc_mem_unmap(uc, vaddr, align(memsz, PAGE_SIZE))) != UC_ERR_OK) {
    printf("uc_mem_unmap() returned error %u (%s)\n", err, uc_strerror(err));
    return NULL;
  }
  if ((err = uc_mem_map_ptr(
      uc, vaddr, align(memsz, PAGE_SIZE), perms, p)) != UC_ERR_OK) {
    printf("uc_mem_map_ptr() returned error %u (%s)\n", err, uc_strerror(err));
    return NULL;
  }

  return p;
}

static void fp_get(void *user, void *dest, uint32_t offs, uint32_t len)
{
  FILE *fp = user;
  fseek(fp, offs, SEEK_SET);
  fread(dest, len, 1, user);
}

static inline const char *mem_type_str(uc_mem_type t)
{
  switch (t) {
    case UC_MEM_READ: return "read";
    case UC_MEM_WRITE: return "write";
    case UC_MEM_FETCH: return "fetch";
    case UC_MEM_READ_UNMAPPED: return "read unmapped";
    case UC_MEM_WRITE_UNMAPPED: return "write unmapped";
    case UC_MEM_FETCH_UNMAPPED: return "fetch unmapped";
    case UC_MEM_READ_PROT: return "read protected";
    case UC_MEM_WRITE_PROT: return "write protected";
    case UC_MEM_FETCH_PROT: return "fetch protected";
    default: return "--";
  }
}

static void handler_unmapped(
  uc_engine *uc, uc_mem_type type,
  uint64_t address, int size, int64_t value, void *user_data)
{
  if (type == UC_MEM_FETCH_UNMAPPED && address == 0) {
    printf("Done!\n");
    return;
  }
  printf("Invalid memory access 0x%08x (type = %s, value = 0x%08x)\n",
    (uint32_t)address, mem_type_str(type), (uint32_t)value);
}

static void step_context(uc_context *ctx)
{
  uc_err err;

  uc_context_restore(uc, ctx);
  uint32_t pc;
  uc_reg_read(uc, UC_ARM_REG_PC, &pc);

  update_tick();

  // XXX: uc_emu_continue()?
  if ((err = uc_emu_start(uc, pc, 0, 100000, 0)) != UC_ERR_OK) {
    printf("uc_emu_start() returned error %u (%s)\n", err, uc_strerror(err));
    uc_reg_read(uc, UC_ARM_REG_PC, &pc);
    printf("PC = 0x%08x\n", pc);
    exit(1);
  }

  uc_context_save(uc, ctx);
}

static bool render_sem; // false denotes that drawing commands are issued
static bool vsync_sem;  // false denotes that v-sync has triggered

void *vsync_fn(void *_unused)
{
  while (1) {
    if (!__atomic_test_and_set(&render_sem, __ATOMIC_RELAXED)) {
      glfwSwapBuffers(window);
      usleep(8000);
      __atomic_clear(&vsync_sem, __ATOMIC_RELAXED);
    }
    usleep(1000);
  }
}

void emu()
{
  uc_err err;

  // Initialize Unicorn
  if ((err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc)) != UC_ERR_OK) {
    printf("uc_open() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  // Map memory
  if ((err = uc_mem_map(
      uc, MEM_START_O, MEM_SIZE_O, UC_PROT_READ | UC_PROT_WRITE)) != UC_ERR_OK) {
    printf("uc_mem_map() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  // Enable VFP
  // ref. ../sys/startup.S
  uint32_t val;
  if ((err = uc_reg_read(uc, UC_ARM_REG_C1_C0_2, &val)) != UC_ERR_OK) {
    printf("uc_reg_read() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }
  val |= 0xf00000;  // Single & double precision
  if ((err = uc_reg_write(uc, UC_ARM_REG_C1_C0_2, &val)) != UC_ERR_OK) {
    printf("uc_reg_write() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }
  val = 0x40000000; // Set EN bit
  if ((err = uc_reg_write(uc, UC_ARM_REG_FPEXC, &val)) != UC_ERR_OK) {
    printf("uc_reg_write() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  // Parse and load ELF
  FILE *fp = fopen("user/a.out", "r");
  if (fp == NULL) {
    printf("Unable to open file\n");
    return;
  }
  elf_addr entry;
  uint8_t elfret = elf_load(fp_get, alloc, fp, &entry);
  fclose(fp);
  if (elfret != 0) {
    printf("elf_load() returned error %u\n", elfret);
    return;
  }

  // Add hooks
  uc_hook hook_mem, hook_syscall;
  uc_hook_add(uc, &hook_mem, UC_HOOK_MEM_INVALID, handler_unmapped, NULL, 1, 0);
  uc_hook_add(uc, &hook_syscall, UC_HOOK_INTR, handler_syscall, NULL, 1, 0);

  // Initialize graphics
  v3d_init();

  // Initialize syscalls
  syscalls_init();
  routine_id = -1;
  memset(routine_pc, 0, sizeof routine_pc);

  // Initialize stack pointer
  uint32_t initial_sp = MEM_END_O;
  uint32_t initial_lr = 0x0;
  uc_reg_write(uc, UC_ARM_REG_SP, &initial_sp);
  uc_reg_write(uc, UC_ARM_REG_LR, &initial_lr);

  // Execute initialization routine
  initialize_tick();
  update_tick();
  update_input();

  printf("Entry 0x%08x\n", entry);
  if ((err = uc_emu_start(uc, entry, 0, 1000000, 0)) != UC_ERR_OK) {
    printf("uc_emu_start() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  printf("Routine addresses: 0x%08x 0x%08x 0x%08x 0x%08x\n",
    routine_pc[0], routine_pc[1], routine_pc[2], routine_pc[3]);

  for (int i = 0; i < 4; i++) {
    uc_context_alloc(uc, &routine_ctx[i]);
    uint32_t sp = MEM_END_O - i * STACK_SIZE;
    uint32_t lr = 0;
    uint32_t pc = routine_pc[i];
    uc_reg_write(uc, UC_ARM_REG_SP, &sp);
    uc_reg_write(uc, UC_ARM_REG_LR, &lr);
    uc_reg_write(uc, UC_ARM_REG_PC, &pc);
    uc_context_save(uc, routine_ctx[i]);
  }

  // Execute loop
  uint64_t last_poll = 0, last_comp = 0;
  req_flags = 0xf;
  __atomic_test_and_set(&render_sem, __ATOMIC_RELAXED);
  __atomic_test_and_set(&vsync_sem, __ATOMIC_RELAXED);

  uint64_t req_mask = 0;
  for (int i = 0; i < 4; i++)
    if (routine_pc[i] != 0) req_mask |= (1 << i);

  pthread_t vsync_thread;
  int err_i;
  if ((err_i = pthread_create(&vsync_thread, NULL, vsync_fn, NULL)) != 0) {
    printf("pthread_create() returned error %i (%s)\n", err_i, strerror(err_i));
    return;
  }
  if ((err_i = pthread_detach(vsync_thread)) != 0) {
    printf("pthread_detach() returned error %i (%s)\n", err_i, strerror(err_i));
    return;
  }

  while (1) {
    if ((req_flags & req_mask) == 0) {
      usleep(500);
    } else for (int i = 3; i >= 0; i--) {
      if (routine_pc[i] != 0 && (req_flags & (1 << i))) {
        update_tick();
        routine_id = i;
        step_context(routine_ctx[i]);

        if (i == 0 && !(req_flags & (1 << i))) {
          // Signal that rendering commands have been issued
          __atomic_clear(&render_sem, __ATOMIC_RELAXED);
        }
      }
    }

    update_tick();

    if (!__atomic_test_and_set(&vsync_sem, __ATOMIC_RELAXED))
      req_flags |= (1 << 0);

    if (audio_pending())
      req_flags |= (1 << 1);

    if (app_tick - last_poll >= 3000) {
      glfwPollEvents();
      if (glfwWindowShouldClose(window)) break;
      update_input();
      last_poll = app_tick - app_tick % 3000;
      req_flags |= (1 << 2);
    }

    if (last_comp != app_tick * 240 / 1000000) {
      last_comp = app_tick * 240 / 1000000;
      req_flags |= (1 << 3);
    }

  /*
    for (int i = 3; i >= 0; i--) if (routine_pc[i] != 0) {
      if (i == 0 && app_tick - last_bufswp < 12000) continue;
      if (i == 1 && !audio_pending()) continue;
      step_context(routine_ctx[i]);

      if (i == 0) {
        render();
        update_tick();
        last_bufswp = app_tick;
      }
    }

    update_tick();
    if (app_tick - last_poll >= 3000) {
      last_poll = app_tick;
      glfwPollEvents();
      if (glfwWindowShouldClose(window)) break;
      update_input();
    }
  */
  }
}

int main()
{
  setup_glfw();
  setup_audio();
  emu();
  return 0;
}
