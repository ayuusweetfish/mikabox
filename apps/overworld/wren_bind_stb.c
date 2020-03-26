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
  const char *path = wrenGetSlotString(vm, 1);

  int buf_len;
  unsigned char *buf = read_file(path, &buf_len);
  if (buf == NULL) {
    printf("Cannot open file %s for reading\n", path);
    wrenSetSlotNull(vm, 0);
    return;
  }

  int w, h;
  unsigned char *pix = stbi_load_from_memory(buf, buf_len, &w, &h, NULL, 4);
  if (pix == NULL) {
    printf("Cannot read valid image from file %s\n", path);
    free(buf);
    wrenSetSlotNull(vm, 0);
    return;
  }

  unsigned tex = gfx_tex_create(w, h);
  gfx_tex_update(tex, pix, v3d_tex_fmt_rgba);

  printf("Loaded image %s (%dx%d)\n", path, w, h);
  free(buf);

  // Return [tex, w, h]
  wrenEnsureSlots(vm, 2);
  wrenSetSlotNewList(vm, 0);
  wrenSetSlotDouble(vm, 1, tex);
  wrenInsertInList(vm, 0, 0, 1);
  wrenSetSlotDouble(vm, 1, w);
  wrenInsertInList(vm, 0, 1, 1);
  wrenSetSlotDouble(vm, 1, h);
  wrenInsertInList(vm, 0, 2, 1);
}

WrenForeignMethodFn wren_bind_stb(WrenVM *vm, const char *module,
  const char *class_name, bool is_static, const char *signature)
{
  if (is_static && strcmp(class_name, "Stb") == 0) {
    if (strcmp(signature, "loadImage(_)") == 0) return load_image;
  }

  return NULL;
}
