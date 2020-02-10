#include "mikabox.h"
#include "wren.h"
#include <string.h>

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
    wrenEnsureSlots(vm, 2); \
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

// wren_bind.c
WrenForeignMethodFn wren_bind_method(WrenVM *vm, const char *module,
  const char *class_name, bool is_static, const char *signature);

void main()
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = wren_write;
  config.errorFn = wren_error;
  config.bindForeignMethodFn = wren_bind_method;

  vm = wrenNewVM(&config);

  int f = fil_open("main.wren", FA_READ);
  int len = fil_size(f);
  char *src = malloc(len + 1);
  if (src == NULL) {
    printf("Cannot allocate enough memory\n");
    fil_close(f); while (1) { }
  }
  if (fil_read(f, src, len) < len) {
    printf("File read incomplete\n");
    fil_close(f); while (1) { }
  }
  src[len] = '\0';

  fil_close(f);

  WrenInterpretResult result = wrenInterpret(
    vm, "mikabox_app_module", src);
  free(src);

  // Trap on failure
  if (result != WREN_RESULT_SUCCESS) while (1) { }

  mika_rout(draw, synth, event, update);
  mika_yield(1);
}
