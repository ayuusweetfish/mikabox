#include "emu.h"
#include "elf.h"
#include "swi.h"
#include "syscalls.h"
#include "v3d_wrapper.h"
#include "ff_wrapper.h"
#include "audio_wrapper.h"
#include "../ker/input.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "miniaudio/miniaudio.h"
#include "timer_lib/timer.h"
#include "unicorn/unicorn.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MEM_START_O 0x40000000
#define MEM_SIZE_O  0x01000000  // 16 MiB
#define MEM_END_O   (MEM_START_O + MEM_SIZE_O)
#define MEM_START_A 0x80000000
#define MEM_SIZE_A  0x18000000  // 384 MiB
#define MEM_END_A   (MEM_START_A + MEM_SIZE_A)

#define PAGE_SIZE   0x1000      // 4 KiB
#define STACK_SIZE  0x100000    // 1 MiB

#define WIN_W 800
#define WIN_H 480

bool headless = false;
static const char *cmd_app_path = NULL;

int8_t routine_id;
uint32_t routine_pc[8];
uc_context *routine_ctx[8];

char *program_name = NULL;
bool program_paused = false;
char *request_exec = NULL;
bool request_resume = false;

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
static double usec_rate;

static struct app_timer {
  uint64_t base;
  tick_t start;
} timer_g = { 0 }, timer_o = { 0 }, timer_a = { 0 };

int num_players;
uint64_t player_btns[MAX_PLAYERS];
uint64_t player_axes[MAX_PLAYERS];

static uint64_t player_btns_last[MAX_PLAYERS];

void app_timer_start(struct app_timer *t)
{
  t->start = timer_current();
}

uint64_t app_timer_update(struct app_timer *t)
{
  tick_t diff = timer_current() - t->start;
  return (tick_t)(diff * usec_rate) + t->base;
}

void app_timer_pause(struct app_timer *t)
{
  t->base = app_timer_update(t);
}

void setup_timer()
{
  timer_lib_initialize();
  tick_t tps = timer_ticks_per_second();
  usec_rate = 1e6 / tps;

  app_timer_start(&timer_g);
}

static void update_input()
{
  // TODO: Multiple gamepads
  num_players = 1;
  player_btns[0] =
    BTN_BIT(U, glfwGetKey(window, GLFW_KEY_UP), 0) |
    BTN_BIT(D, glfwGetKey(window, GLFW_KEY_DOWN), 0) |
    BTN_BIT(L, glfwGetKey(window, GLFW_KEY_LEFT), 0) |
    BTN_BIT(R, glfwGetKey(window, GLFW_KEY_RIGHT), 0) |
    BTN_BIT(A, glfwGetKey(window, GLFW_KEY_C), 0) |
    BTN_BIT(B, glfwGetKey(window, GLFW_KEY_X), 0) |
    BTN_BIT(X, glfwGetKey(window, GLFW_KEY_Z), 0) |
    BTN_BIT(Y, glfwGetKey(window, GLFW_KEY_LEFT_SHIFT), 0) |
    BTN_BIT(U, glfwGetKey(window, GLFW_KEY_W), 0) | // Alternative set of keys
    BTN_BIT(D, glfwGetKey(window, GLFW_KEY_S), 0) |
    BTN_BIT(L, glfwGetKey(window, GLFW_KEY_A), 0) |
    BTN_BIT(R, glfwGetKey(window, GLFW_KEY_D), 0) |
    BTN_BIT(A, glfwGetKey(window, GLFW_KEY_K), 0) |
    BTN_BIT(B, glfwGetKey(window, GLFW_KEY_L), 0) |
    BTN_BIT(X, glfwGetKey(window, GLFW_KEY_J), 0) |
    BTN_BIT(Y, glfwGetKey(window, GLFW_KEY_I), 0) |
    BTN_BIT(L1, glfwGetKey(window, GLFW_KEY_Q), 0) |
    BTN_BIT(R1, glfwGetKey(window, GLFW_KEY_TAB), 0) |
    BTN_BIT(L2, glfwGetKey(window, GLFW_KEY_P), 0) |
    BTN_BIT(R2, glfwGetKey(window, GLFW_KEY_LEFT_BRACKET), 0) |
    BTN_BIT(L3, glfwGetKey(window, GLFW_KEY_V), 0) |
    BTN_BIT(R3, glfwGetKey(window, GLFW_KEY_M), 0) |
    BTN_BIT(START, glfwGetKey(window, GLFW_KEY_ENTER), 0) |
    BTN_BIT(OPTN, glfwGetKey(window, GLFW_KEY_APOSTROPHE), 0) |
    BTN_BIT(META, glfwGetKey(window, GLFW_KEY_SPACE), 0) |
    BTN_BIT(AUX, glfwGetKey(window, GLFW_KEY_SLASH), 0);

  if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
      player_btns[0] |=
        BTN_BIT(U, state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP], 0) |
        BTN_BIT(D, state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN], 0) |
        BTN_BIT(L, state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT], 0) |
        BTN_BIT(R, state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT], 0) |
        BTN_BIT(A, state.buttons[GLFW_GAMEPAD_BUTTON_A], 0) |
        BTN_BIT(B, state.buttons[GLFW_GAMEPAD_BUTTON_B], 0) |
        BTN_BIT(X, state.buttons[GLFW_GAMEPAD_BUTTON_X], 0) |
        BTN_BIT(Y, state.buttons[GLFW_GAMEPAD_BUTTON_Y], 0) |
        BTN_BIT(L1, state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER], 0) |
        BTN_BIT(R1, state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER], 0) |
        BTN_BIT(L2, state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 1e-5, 0) |
        BTN_BIT(R2, state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 1e-5, 0) |
        BTN_BIT(L3, state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB], 0) |
        BTN_BIT(R3, state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB], 0) |
        BTN_BIT(START, state.buttons[GLFW_GAMEPAD_BUTTON_BACK], 0) |
        BTN_BIT(START, state.buttons[GLFW_GAMEPAD_BUTTON_START], 0) |
        BTN_BIT(OPTN, state.buttons[GLFW_GAMEPAD_BUTTON_GUIDE], 0);
  }
}

// Unicorn Engine

static uc_engine *uc;

static inline uint32_t align(uint32_t addr, uint32_t align)
{
  return (addr + align - 1) & ~(align - 1);
}

static void *app_allocated[] = { NULL };
static int app_alloc_count = 0;

static void *alloc(elf_word vaddr, elf_word memsz, elf_word flags)
{
  void *p = malloc(memsz);
  // Application memory
  if (vaddr >= 0x80000000) app_allocated[app_alloc_count++] = p;

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

static void init_application(const char *path, bool overworld)
{
  uc_err err;

  // Unmap memory
  uint32_t mem_start = (overworld ? MEM_START_O : MEM_START_A);
  uint32_t mem_size = (overworld ? MEM_SIZE_O : MEM_SIZE_A);
  if ((err = uc_mem_unmap(uc, mem_start, mem_size)) != UC_ERR_OK &&
      err != UC_ERR_NOMEM)
  {
    printf("uc_mem_unmap() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }
  for (int i = 0; i < app_alloc_count; i++) free(app_allocated[i]);
  app_alloc_count = 0;
  // Remap memory
  if ((err = uc_mem_map(
      uc, mem_start, mem_size, UC_PROT_READ | UC_PROT_WRITE)) != UC_ERR_OK) {
    printf("uc_mem_map() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  // Parse and load ELF
  FILE *fp = fopen(path, "r");
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

  int bank = (overworld ? 0 : 4);
  routine_id = (overworld ? -1 : -2);
  for (int i = 0; i < 4; i++) routine_pc[bank + i] = 0;

  // Initialize stack pointer
  uint32_t initial_sp = (overworld ? MEM_END_O : MEM_END_A);
  uint32_t initial_lr = 0x0;
  uc_reg_write(uc, UC_ARM_REG_SP, &initial_sp);
  uc_reg_write(uc, UC_ARM_REG_LR, &initial_lr);

  // Execute initialization routine
  struct app_timer *t = overworld ? &timer_o : &timer_a;
  t->base = 0;
  app_timer_start(t);
  update_input();

  printf("Entry 0x%08x\n", entry);
  if ((err = uc_emu_start(uc, entry, 0, 1000000, 0)) != UC_ERR_OK) {
    printf("uc_emu_start() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  printf("Routine addresses: 0x%08x 0x%08x 0x%08x 0x%08x\n",
    routine_pc[bank + 0], routine_pc[bank + 1], routine_pc[bank + 2], routine_pc[bank + 3]);

  for (int i = 0; i < 4; i++) {
    if (routine_ctx[bank + i] == NULL)
      uc_context_alloc(uc, &routine_ctx[bank + i]);
    uint32_t sp = initial_sp - i * STACK_SIZE;
    uint32_t lr = 0;
    uint32_t pc = routine_pc[bank + i];
    uc_reg_write(uc, UC_ARM_REG_SP, &sp);
    uc_reg_write(uc, UC_ARM_REG_LR, &lr);
    uc_reg_write(uc, UC_ARM_REG_PC, &pc);
    uc_context_save(uc, routine_ctx[bank + i]);
  }
}

static void step_context(uc_context *ctx)
{
  uc_err err;

  uc_context_restore(uc, ctx);
  uint32_t pc;
  uc_reg_read(uc, UC_ARM_REG_PC, &pc);

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

  // Add hooks
  uc_hook hook_mem, hook_syscall;
  uc_hook_add(uc, &hook_mem, UC_HOOK_MEM_INVALID, handler_unmapped, NULL, 1, 0);
  uc_hook_add(uc, &hook_syscall, UC_HOOK_INTR, handler_syscall, NULL, 1, 0);

  // Initialize graphics
  v3d_init();

  // Initialize syscalls
  syscalls_init();

  // Execute loop
  uint64_t last_poll = 0, last_comp = 0;
  req_flags = 0xf;
  __atomic_test_and_set(&render_sem, __ATOMIC_RELAXED);
  __atomic_test_and_set(&vsync_sem, __ATOMIC_RELAXED);

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

  if (headless) {
    init_application(cmd_app_path, false);
    program_name = "Some Program";
  } else {
    init_application(cmd_app_path, true);
  }

  uint64_t req_mask = 0xf;

  while (1) {
    if (!(player_btns_last[0] & BTN_START) && (player_btns[0] & BTN_START)) {
      if (!program_paused) {
        // Pause
        program_paused = true;
        app_timer_pause(&timer_a);
        app_timer_start(&timer_o);
      } else if (headless) {
        // Resume
        request_resume = true;
      }
    }
    if (program_name != NULL && request_resume) {
      // Resume
      request_resume = false;
      program_paused = false;
      app_timer_pause(&timer_o);
      app_timer_start(&timer_a);
    }

    memcpy(player_btns_last, player_btns, sizeof player_btns);

    int bank = ((program_name == NULL || program_paused) ? 0 : 4);
    struct app_timer *timer = (bank == 0 ? &timer_o : &timer_a);

    if ((req_flags & req_mask) == 0) {
      usleep(500);
    } else for (int i = 3; i >= 0; i--) {
      if (routine_pc[bank + i] != 0 && (req_flags & (1 << i))) {
        app_tick = app_timer_update(timer);

        routine_id = bank + i;
        step_context(routine_ctx[bank + i]);

        if (i == 0 && !(req_flags & (1 << i))) {
          // Signal that rendering commands have been issued
          __atomic_clear(&render_sem, __ATOMIC_RELAXED);
        }
      }
    }

    // Process requests
    if (request_exec != NULL) {
      char *full_path;
      asprintf(&full_path, "%s%s", fs_root, request_exec);
      printf("execute: %s\n", full_path);
      init_application(full_path, false);
      free(request_exec);
      free(full_path);
      request_exec = NULL;
      program_name = "Some Program";
      program_paused = false;
      app_timer_pause(&timer_o);
      app_timer_start(&timer_a);
    }

    app_timer_update(timer);
    uint64_t global_tick = app_timer_update(&timer_g);

    if (!__atomic_test_and_set(&vsync_sem, __ATOMIC_RELAXED))
      req_flags |= (1 << 0);

    if (audio_pending())
      req_flags |= (1 << 1);

    if (global_tick - last_poll >= 3000) {
      glfwPollEvents();
      if (glfwWindowShouldClose(window)) break;
      update_input();
      last_poll = global_tick - global_tick % 3000;
      req_flags |= (1 << 2);
    }

    if (last_comp != global_tick * 240 / 1000000) {
      last_comp = global_tick * 240 / 1000000;
      req_flags |= (1 << 3);
    }
  }
}

bool parse_args(int argc, char *argv[])
{
  headless = false;
  cmd_app_path = NULL;
  fs_root = NULL;

  int ind = 1;

  if (ind >= argc) return false;
  if (strcmp(argv[ind], "-a") == 0) {
    headless = true;
    ind++;
  }

  if (ind >= argc) return false;
  cmd_app_path = argv[ind++];

  if (ind < argc) {
    size_t len = strlen(argv[2]);
    if (len == 0 || argv[2][len - 1] != '/') {
      char *s = (char *)malloc(len + 2);
      strcpy(s, argv[2]);
      s[len] = '/';
      s[len + 1] = '\0';
      fs_root = s;
    } else {
      fs_root = argv[2];
    }
  } else {
    // Extract directory path from executable path
    int pos = -1;
    for (int i = 0; cmd_app_path[i] != '\0'; i++)
      if (cmd_app_path[i] == '/') pos = i;
    if (pos == -1) {
      fs_root = "./";
    } else {
      char *s = (char *)malloc(pos + 2);
      memcpy(s, cmd_app_path, pos + 1);
      s[pos + 1] = '\0';
      fs_root = s;
    }
  }

  return true;
}

int main(int argc, char *argv[])
{
  if (!parse_args(argc, argv)) {
    printf("usage: %s [-a] <executable> [<file system root>]\n", argv[0]);
    printf("  -a    Headless: run application without overworld\n");
    return 0;
  }

  printf("Starting emulation:\n");
  printf("Executable:       %s\n", cmd_app_path);
  printf("File system root: %s\n", fs_root);

  setup_glfw();
  setup_audio();
  setup_timer();
  emu();

  return 0;
}
