# awk -F, -v output=<c|wren|wren_bind_1|wren_bind_2> -f syscalls.awk syscalls.txt
# awk -F, -v output=wren_bind_1 -f syscalls.awk syscalls.txt > wren_bind.c && awk -F, -v output=wren_bind_2 -f syscalls.awk syscalls.txt >> wren_bind.c

BEGIN {
  type_c["_"] = "void"
  type_c["u32"] = "uint32_t"
  type_c["u64"] = "uint64_t"
  type_c["ptr"] = "void *"
  type_c["cptr"] = "const void *"

  if (output == "wren_bind_1") {
    printf("#include \"mikabox.h\"\n")
    printf("#include \"wren.h\"\n")
    printf("#include <string.h>\n\n")
  } else if (output == "wren_bind_2") {
    printf("WrenForeignMethodFn wren_bind_method(WrenVM *vm, const char *module,\n")
    printf("  const char *class_name, bool is_static, const char *signature)\n")
    printf("{\n")
    printf("  if (is_static && strcmp(class_name, \"Mikabox\") == 0) {\n")
  }
}

function underscore_to_camel(s,   result, n, i, last_und) {
  result = ""
  n = split(s, s, "")
  last_und = 0
  for (i = 1; i <= length(s); i++) {
    if (s[i] == "_") {
      last_und = 1
    } else if (last_und) {
      last_und = 0
      result = result toupper(s[i])
    } else {
      result = result s[i]
    }
  }
  return result
}

$1 ~ /^ *[0-9]+ *$/ {
  n = split($2, def, " ")

  if (n == 3 && def[1] == "==" && def[3] == "==") {
    # Scope change
    scope_offset = $1
    scope = def[2]
  } else if (NF >= 2) {
    # Definition
    type[0] = def[1]
    name[0] = def[2]

    for (i = 3; i <= NF; i++)
      if (split($i, arg, " ") == 2) {
        type[i - 2] = arg[1]
        name[i - 2] = arg[2]
      }
    argc = NF - 2

    if (output == "c") {
      printf("static inline %s%s_%s(", type_c[type[0]], scope, name[0])
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(", ")
        printf("%s%s%s", type_c[type[i]], (type[i] ~ "ptr") ? "" : " ", name[i])
      }
      printf(") { ")
      if (type[0] != "_") printf("return ")
      printf("syscall%d(", argc)
      for (i = 1; i <= argc; i++) {
        if (type[i] ~ "ptr") printf("(uint32_t)")
        printf("%s, ", name[i])
      }
      printf("%d); }\n", scope_offset + $1)
    } else if (output == "wren") {
      printf("foreign static %s(",
        underscore_to_camel((scope == "mika" ? "" : scope "_") name[0]))
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(", ")
        printf("%s", name[i])
      }
      printf(")\n")
    } else if (output == "wren_bind_1") {
      printf("static void wren_%s(WrenVM *vm)\n", scope "_" name[0])
      printf("{\n")
      for (i = 1; i <= argc; i++) {
        printf("  if (wrenGetSlotType(vm, %d) != WREN_TYPE_NUM)\n", i)
        printf("    printf(\"Argument %d has incorrect type\");\n", i)
        printf("  double %s = wrenGetSlotDouble(vm, %d);\n\n", name[i], i)
      }
      printf("  ")
      if (type[0] != "_") printf("double ret = (double)")
      printf("%s_%s(", scope, name[0])
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(", ")
        printf("(%s)(uint32_t)%s", type_c[type[i]], name[i])
      }
      printf(");\n")
      if (type[0] != "_") printf("  wrenSetSlotDouble(vm, 0, ret);\n")
      printf("}\n\n")
    } else if (output == "wren_bind_2") {
      printf("    if (strcmp(signature, \"%s(",
        underscore_to_camel((scope == "mika" ? "" : scope "_") name[0]))
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(",")
        printf("_")
      }
      printf(")\") == 0) ")
      printf("return wren_%s;\n", scope "_" name[0])
    }
  }
}

END {
  if (output == "wren_bind_2") {
    printf("  }\n")
    printf("\n")
    printf("  return NULL;\n")
    printf("}\n")
  }
}
