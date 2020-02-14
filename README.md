# Mikabox

:construction: A modern gaming console from the ground up. Successor of [MIKAN][].

[MIKAN]: https://github.com/kawa-yoiko/MIKAN/

This project aims to implement a simple cooperatively-multitasking operating
system on Raspberry Pi Zero for multimedia applications (games, demoscenes,
interactive applications) and a simple development toolchain, including
multi-language support (C, Wren, Rust and more) and a cross-platform emulator.
A few games and emulators of other consoles will be ported.

Status: mostly usable, overworld menu still in progress, docs are being
completed. Expect breaking changes every week.

## Building

#### Kernel

```
cd sys
make libs
make
```

#### Emulator

```
cd emu
make lib
make
```

#### Tetris

```
cd apps/tetris
make run    # Build the emulator first
```
