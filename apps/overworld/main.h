#ifndef _Overworld_h_
#define _Overworld_h_

#include "mikabox.h"
#include "wren.h"

char *read_file(const char *path);

// wren_bind_mikabox.c

extern const char *wren_def_mikabox;

WrenForeignMethodFn wren_bind_mikabox(WrenVM *vm, const char *module,
  const char *class_name, bool is_static, const char *signature);

// wren_bind_stb.c

extern const char *wren_def_stb;

WrenForeignMethodFn wren_bind_stb(WrenVM *vm, const char *module,
  const char *class_name, bool is_static, const char *signature);


#endif
