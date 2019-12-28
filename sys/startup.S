.section ".text.startup"

.global startup
startup:
  b     sys_main

_loop:
  # In case kernel returns O_O
  b     _loop
