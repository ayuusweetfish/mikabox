#include "mikabox.h"
#include "wren.h"
#include <string.h>

static char *read_file(const char *path)
{
  int f = fil_open(path, FA_READ);
  int len = fil_size(f);
  char *buf = malloc(len + 1);
  if (buf == NULL) {
    printf("Cannot allocate enough memory\n");
    fil_close(f); return NULL;
  }
  if (fil_read(f, buf, len) < len) {
    printf("File read incomplete\n");
    fil_close(f); return NULL;
  }
  buf[len] = '\0';

  fil_close(f);
  return buf;
}

static WrenVM *vm;

#define wren_routine(_name) \
void _name() \
{ \
  wrenEnsureSlots(vm, 2); \
  wrenGetVariable(vm, "mikabox_app_module", #_name, 0); \
  WrenHandle *fiber = wrenGetSlotHandle(vm, 0); \
  WrenHandle *method = wrenMakeCallHandle(vm, "call()"); \
 \
  while (1) { \
    wrenSetSlotHandle(vm, 0, fiber); \
    WrenInterpretResult result = wrenCall(vm, method); \
    if (result != WREN_RESULT_SUCCESS) while (1) { } \
    bool b = wrenGetSlotBool(vm, 0); \
    mika_yield((int)b); \
  } \
}

wren_routine(draw)
wren_routine(synth)
wren_routine(event)
wren_routine(update)

static void wren_write(WrenVM *vm, const char *text)
{
  static char buf[256];
  static int ptr = 0;

  for (const char *p = text; *p != '\0'; p++) {
    if (*p == '\n' || ptr == sizeof(buf) - 1) {
      buf[ptr] = '\0';
      mika_log(1, buf);
      ptr = 0;
    }
    if (*p != '\n') buf[ptr++] = *p;
  }
}

static void wren_error(WrenVM *vm, WrenErrorType type,
  const char *module, int line, const char *message)
{
  printf("%s:%d: %s\n", module, line, message);
}

extern const char *wren_mikabox_def;

static char *wren_load_module(WrenVM *vm, const char *name)
{
  // asprintf and strdup are non-standard

  if (strcmp(name, "mikabox") == 0) {
    size_t length = strlen(wren_mikabox_def);
    char *copy = malloc(length + 1);
    memcpy(copy, wren_mikabox_def, length + 1);
    return copy;
  }

  size_t length = strlen(name);
  char *path = malloc(length + 7);
  sprintf(path, "/%s.wren", name);
  char *buf = read_file(path);

  free(path);
  return buf;
}

// wren_bind.c
WrenForeignMethodFn wren_bind_method(WrenVM *vm, const char *module,
  const char *class_name, bool is_static, const char *signature);

void main()
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = wren_write;
  config.errorFn = wren_error;
  config.loadModuleFn = wren_load_module;
  config.bindForeignMethodFn = wren_bind_method;
  config.initialHeapSize = 4 << 20;

  vm = wrenNewVM(&config);

  char *src = read_file("/main.wren");
  if (src == NULL) while (1) { }

  WrenInterpretResult result = wrenInterpret(
    vm, "mikabox_app_module", src);
  free(src);

  // Trap on failure
  if (result != WREN_RESULT_SUCCESS) while (1) { }

  mika_rout(draw, synth, event, update);
  mika_yield(1);
}
