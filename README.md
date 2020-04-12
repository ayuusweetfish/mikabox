# Mikabox

:construction: A modern gaming console from the ground up. Successor of [MIKAN][].

[MIKAN]: https://github.com/kawa-yoiko/MIKAN/

This project aims to implement a simple cooperatively-multitasking operating
system on Raspberry Pi Zero for multimedia applications (games, demoscenes,
interactive devices) and a simple development toolchain, including
multi-language support (C, Wren, Rust and more) and a cross-platform emulator.
A few games and emulators of other consoles will be ported.

Status: basic functionalities all done, a Tetris clone working well; overworld
menu still in progress, docs are being completed. Not moving so fast recently,
but expect breaking changes once we get back to it. Appreciation will be a big
motivation (> <)

## Building

#### Kernel

```
cd ker
make libs && make
```

#### Emulator

```
cd emu
make libs && make
```

#### Tetris

```
cd apps/tetris
make run    # Build the emulator first
```

#### Documentation

```
cd docs/site
hugo serve
```
