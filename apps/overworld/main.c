#include "mikabox.h"
#include "wren.h"

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

void crt_init()
{
  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
}

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
  crt_init();

  mika_log(0, "Hello world!\n");

/*  int *a = malloc(64);
  mika_printf("%p\n", a);
  int *c = malloc(64);
  mika_printf("%p\n", c);
  int *b = realloc(a, 65536);
  mika_printf("%p\n", b);
  while (1) { }*/

  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = &wren_write;
  config.errorFn = *wren_error;

  WrenVM *vm = wrenNewVM(&config);

  char src[1048576];
  int f = fil_open("main.wren", FA_READ);
  int len = fil_size(f);
  if (len >= sizeof src) {
    mika_printf("File too long (%u bytes)\n", len);
    goto done;
  }
  if (fil_read(f, src, len) < len) {
    mika_printf("File read incomplete\n");
    goto done;
  }
  src[len] = '\0';

done:
  fil_close(f);

  WrenInterpretResult result = wrenInterpret(
    vm, "mikabox_app_module", src);
  mika_printf("result: %d\n", (int)result);

  wrenFreeVM(vm);

  syscall(0, draw, synth, event, update);
  mika_yield(1);
}
