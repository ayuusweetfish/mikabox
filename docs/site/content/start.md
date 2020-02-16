---
title: "Getting started"
weight: 10
---

## Setting up

Download [the release package], or alternatively [build from source].

[the release package]: #
[build from source]: #building-from-source

### The hardware

Copy the contents of the __firmware/__ directory inside the package to a microSD
card formatted as an FAT16 or FAT32 filesystem. Load the card onto a Raspberry
Pi Zero, power on the device and it should boot into the Overworld.

### The emulator

The emulator is in __emu/\<platform\>/__ and can be run with the command
`./emu <path to firmware/a.out>`. Invoke `./emu -h` for a complete usage guide.

Alternatively, copy the emulator executable into __firmware/__ and double-click
it in the file manager.

### Building from source

```
git clone --recursive https://github.com/kawa-yoiko/mikabox
```

#### Requirements

The following are needed in the building process; none of them is necessary for
running.

- [GNU Arm Embedded Toolchain] (preferbly added to `$PATH`)
- GNU Make
- (kernel only) Node.js
- (kernel only) Normal GCC/Clang installation on the host
- (emulator only) CMake
- (emulator only) OpenGL development headers and libraries

[GNU Arm Embedded Toolchain]: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

#### Kernel

```
cd sys
make libs && make
```

#### Emulator

```
cd emu
make libs && make
```

#### Applications

```
cd apps/<name>
make run    # Build the emulator first
```

## Development with the C language

Make sure to grab the tools:

- [GNU Arm Embedded Toolchain] (preferbly added to `$PATH`)
- GNU Make

Make a copy of the template __hello_mikabox/__ inside the package, enter the
directory and run `make`.

Then run this "Hello World" application with `<path to emu> -a <path to a.out>`.

See [Application development] for further documentations.

[Application development]: #
