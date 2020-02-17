---
title: "Application development"
weight: 20
---

## Programming in C

Inside the __hello_mikabox/__ directory is the "Hello World" program.

The following provides a description from a C programmer's perspective. For
internal details including the C runtime, refer to the [Inside the box] section.

[Inside the box]: #inside-the-box

### Prerequisites

So far, Mikabox has not been ideal for a very first exploration in programming.
However, we believe with efforts to create a more introductory tutorial it can
be made possible.

The reader is expected to get familiar with a Unix-like operating system, be
able to work with command lines, and be able to write simple programs in C. No
experience with operating system internals is assumed.

### Overview

An application in Mikabox has four threads running simultaneously: **draw**,
**synth**, **event** and **update**, which can be considered four parallel
loops, each taking care of one aspect of the application -- graphics, audio,
event and periodic updates respectively. They share a common pool of resources
including memory and various handles acquired from the system.

Unlike most multitasking systems seen today, Mikabox employs **cooperative
multitasking** (also known as **non-preemptive multitasking**), which means a
thread of execution should explicitly signal that it has done its work and gives
up control -- this operation is called **yielding**. A program needs not worry
about being interrupted at unpredictable times, but should always take up the
duty to yield at reasonable times to time in order not to block the entire
system.

The entry point of the program is still the `main()` function, which may carry
out initialization work, but its most important obligation is to specify which
four functions to use as the threads. After it yields, the threads will be
spawned by the system and start to run.

### System calls

The header file __mikabox.h__ provides a set of functions for communicating with
the system. These are no different from normal functions, but they actually call
into the system and lets it do the work, hence the name **system calls**.

There are system calls for various purposes. `main()` uses one to yield and one
to point to the threads; the threads uses many more to draw shapes, play sounds,
receive input, read files etc.

Relevant system calls will be covered in the subsections to follow.

## Other languages

Support for other languages is work in progress. Planned are:

- Wren
- Lua
- Rust
- Pascal
- Many more scripting languages

Take a look [into the box] if interested in adopting a new language.

[into the box]: #inside-the-box

## Inside the box

An application is a normal ELF file compiled for ARMv6.

### Memory map

- 0x80000000 -- 0x97bfffff: Application space. From lowest to highest:
  - Code (`.text`)
  - Constants (`.rodata`)
  - Global and static data (`.data`, `.bss`)
  - Heap space
- 0x97c00000 -- 0x97cfffff: Stack space for thread **draw**
- 0x97d00000 -- 0x97dfffff: Stack space for thread **synth**
- 0x97e00000 -- 0x97efffff: Stack space for thread **event**
- 0x97f00000 -- 0x97ffffff: Stack space for thread **update**
