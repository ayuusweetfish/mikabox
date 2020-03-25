#include "main.h"
#include "stb/stb_image.h"
#include <stdbool.h>
#include <string.h>

#define VERBATIM(...) #__VA_ARGS__

const char *wren_def_stb = VERBATIM(
  class Stb {
    foreign static loadImage(path)
  }
);

static void load_image(WrenVM *vm)
{
  if (wrenGetSlotType(vm, 1) != WREN_TYPE_STRING)
    printf("Argument 1 has incorrect type");
  const void *path = wrenGetSlotString(vm, 1);

  printf("path: %s\n", path);
}

WrenForeignMethodFn wren_bind_stb(WrenVM *vm, const char *module,
  const char *class_name, bool is_static, const char *signature)
{
  if (is_static && strcmp(class_name, "Stb") == 0) {
    if (strcmp(signature, "loadImage(_)") == 0) return load_image;
  }

  return NULL;
}
