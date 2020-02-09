#include "mikabox.h"
#include "wren.h"

static WrenVM *vm;

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
  syscall(128, 1, 2, 3, 4);

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
  printf("result: %d\n", (int)result);

  free(src);

  syscall(0, draw, synth, event, update);
  mika_yield(1);
}
