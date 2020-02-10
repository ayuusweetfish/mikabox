# awk -F, -v output=<c|wren> -f syscalls.awk syscalls.txt

BEGIN {
  type_c["_"] = "void "
  type_c["u32"] = "uint32_t "
  type_c["u64"] = "uint64_t "
  type_c["ptr"] = "void *"
  type_c["cptr"] = "const void *"
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
        printf("%s%s", type_c[type[i]], name[i])
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
      printf("foreign %s(",
        underscore_to_camel((scope == "mika" ? "" : scope "_") name[0]))
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(", ")
        printf("%s", name[i])
      }
      printf(")\n")
    }
  }
}
