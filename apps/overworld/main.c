#include "mikabox.h"
#include "wren.h"

static WrenVM *vm;
static WrenHandle *obj_app;
static WrenHandle *fiber_update;

void draw()
{
  while (1) {
    mika_yield(1);
  }
}

void synth()
{
  while (1) {
    mika_yield(1);
  }
}

void event()
{
  while (1) {
    mika_yield(1);
  }
}

void update()
{
  while (1) {
    mika_yield(1);
  }
}

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

void main()
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = &wren_write;
  config.errorFn = *wren_error;

  vm = wrenNewVM(&config);

  int f = fil_open("main.wren", FA_READ);
  int len = fil_size(f);
  char *src = malloc(len);
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

  wrenEnsureSlots(vm, 1);
  wrenGetVariable(vm, "mikabox_app_module", "App", 0);
  WrenHandle *class_app = wrenGetSlotHandle(vm, 0);
  WrenHandle *method_new = wrenMakeCallHandle(vm, "new()");

  wrenSetSlotHandle(vm, 0, class_app);
  result = wrenCall(vm, method_new);
  if (result != WREN_RESULT_SUCCESS) while (1) { }

  obj_app = wrenGetSlotHandle(vm, 0);
  wrenReleaseHandle(vm, class_app);
  wrenReleaseHandle(vm, method_new);

  /*result = wrenInterpret(vm, "mikabox_host_module",
    "var update = Fiber.new {|a| a.update()}\n");
  if (result != WREN_RESULT_SUCCESS) while (1) { }*/

  wrenGetVariable(vm, "mikabox_app_module", "update", 0);
  fiber_update = wrenGetSlotHandle(vm, 0);
  WrenHandle *method_call = wrenMakeCallHandle(vm, "call(_)");

  wrenEnsureSlots(vm, 2);
  for (int i = 0; i < 10; i++) {
    wrenSetSlotHandle(vm, 0, fiber_update);
    wrenSetSlotHandle(vm, 1, obj_app);
    result = wrenCall(vm, method_call);
    if (result != WREN_RESULT_SUCCESS) while (1) { }
  }

  syscall(0, draw, synth, event, update);
  mika_yield(1);
}
