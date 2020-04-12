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

So far, Mikabox has not been ideal for a very first exploration in programming
or graphics. However, we believe with efforts to create a more introductory
tutorial it can be made possible.

The reader is expected to get familiar with a Unix-like operating system, be
able to work with command lines, be able to write simple programs in C, and have
a general idea of a basic modern OpenGL graphics pipeline (vertex/fragment
shaders with attribute arrays and buffer objects). No experience with operating
system internals is assumed.

### Overview

An application in Mikabox has four threads running simultaneously: **draw**,
**synth**, **event** and **update**, which can be considered four parallel
long-running loops, each taking care of one aspect of the application --
graphics, audio, event and periodic updates respectively. They share a common
pool of resources including memory and various handles acquired from the system.

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
into the system and lets it do the work, hence the name **system calls**. On
this page, system calls will be referred to by their C wrapper function names.

Relevant system calls will be covered in the subsections to follow, and a
complete list of system calls can be found [here].

[here]: #TODO

### Entry

`main()` should have return type **void** and should register the threads
through `mika_rout()`. Yielding is done through `mika_yield()`, and in `main()`
yielding means to exit the function and spawn the four threads.

```c
void main()
{
  // Register thread functions. The functions are
  // to be implemented in the following sections
  mika_rout(draw, synth, event, update);
  // Yield. The argument is irrelevant in main()
  // and will be introduced in the next subsection
  mika_yield(1);
}
```

### Scheduling

After `main()` yields, the four threads start running. Each time, one of them is
waken up from where it left off at its last yield, and runs until it yields
again by itself.

A thread can be **idle** or **pending**. At any given time, all pending threads
are invoked in rotation. Threads are set to the pending state on the right
conditions:

- **draw**: at each screen refresh (usually at ~60 Hz)
- **synth**: when an audio data block is requested (usually at ~50 Hz)
- **event**: when an input state has been updated (but not necessarily different, usually at ~100 Hz)
- **update**: once every 1/240 of a second (240 Hz)

Yielding with a non-zero argument moves the thread to the idle state, and an
argument of zero does not. Non-zero is usually used; yielding with zero as the
argument can be used during time-consuming operations to avoid blocking.

### Thread "update"

This is the "clock signal" of the application. Any periodic update goes here:
physics, animation, state recording, etc.

In the "hello" application, updates of physics is done here.

Although it can be assumed that this thread is waken up 240 times per second,
`mika_tick()` is able to provide a more accurate timestamp in microseconds.

```c
void update()
{
  // Initialization
  float vx = 0, vy = 0;
  long long t = mika_tick();
  // -- snip --

  while (1) {
    // Update
    long long t1 = mika_tick();
    float dt = (t1 - t) / 1e6f;
    t = t1;

    // -- snip --
    x += (vx + ax * 0.5) * dt;
    y += (vy + ay * 0.5) * dt;
    // -- snip --

    // Yield
    mika_yield(1);
  }
}
```

### Thread "event"

This is separated from **update** in order to provide a more accurate timestamp
of events when desired. But it is conventional to update global event states in
this thread, through `mika_btns()` and `mika_axes()`.

```c
// Global state used by the update thread
static long long buttons;

void event()
{
  buttons = 0;
  while (1) {
    // A bitmask of buttons of the first player
    buttons = mika_btns(0);
    mika_yield(1);
  }
}
```

### Thread "synth"

This thread should keep generating blocks of audio data and pass then to the
system. Audio data is always stereo with interleaved signed 16-bit integers
at 44.1 kHz.

`aud_blocksize()` returns the number of stereo samples in each block (i.e.
`aud_blocksize() * 2` short integers), and does not change since the start of
the application.

When the block of data is ready, pass it inside `aud_write()` and yield with a
non-zero argument. `aud_write()` should not be called more than once between two
non-zero yields.

```c
void synth()
{
  int block = aud_blocksize();
  // -- snip --

  short buf[block][2];

  while (1) {
    for (int i = 0; i < block; i++) {
      buf[i][0] = buf[i][1] =
        (short)(sinf(/* -- snip -- */) * 16384);
    }
    aud_write(buf);
    mika_yield(1);
  }
}
```

### Thread "draw"

Mikabox currently uses a stripped-down version of the modern OpenGL pipeline,
without vertex shaders (which may be changed in the future!).

A **vertex array** stores the 2D position of a vertex, plus a specified number
of varyings. Coordinates are in pixels, and all values stored in the vertex
array are in IEEE 32-bit floating point format.

Three vertices form a triangle. When drawing, varyings are linearly interpolated
inside.

In the "hello" application, each vertex needs the colour specified in RGB.

```c
// 6 vertices with 3 varyings (R, G, B) each
int va = gfx_varr_create(6, 3);
```

The contents of the vertex array is updated through `gfx_varr_put()`.

```c
// A triangle's 5 parameters: X, Y, and 3 varyings
float p[3][5];

// The first triangle
p[0][0] = x - size;
p[0][1] = y - size;
p[0][2] = 1.0; p[0][3] = 0.6; p[0][4] = 0.6;

p[1][0] = x + size;
p[1][1] = y - size;
p[1][2] = 0.6; p[1][3] = 1.0; p[1][4] = 0.6;

p[2][0] = x - size;
p[2][1] = y + size;
p[2][2] = 0.6; p[2][3] = 0.6; p[2][4] = 1.0;

// Write 3 vertices into the array, starting from
// index 0
gfx_varr_put(va, 0, p, 3);
```

A **uniform array** stores values shared by all vertices. Each uniform is 32-bit
and can either be a 32-bit floating point value, or be half of a 64-bit texture
configuration. Uniforms are not used in the "hello" application.

```c
// 0 uniforms
int ua = gfx_uarr_create(0);
```

A **shader** specifies how the colour is determined from its varyings and
uniforms. For simple opaque colours, there is a built-in shader.

```c
int sh = gfx_shad_create("#C");
```

A **batch** refers to a vertex array, a uniform array, and a shader. This allows
any part of the data to be reused and changed without duplication.

```c
int bat = gfx_bat_create(va, ua, sh);
```

A graphics **context** stores a configuration and a list of draw calls from one
or more batches.

```c
int ctx = gfx_ctx_create();
// -- snip --

while (1) {
  // -- snip --

  // Draw onto the screen, with a background
  // colour in ARGB8888 format
  // Note: gfx_tex_screen() has to be called each
  // time due to buffer swapping of the screen
  gfx_ctx_config(ctx, gfx_tex_screen(), 0xffffeecc);

  // Clear all draw commands
  gfx_ctx_reset(ctx);
  // Use the batch
  gfx_ctx_batch(ctx, bat);
  // Draw 6 vertices (2 triangles) starting from
  // index 0
  gfx_ctx_call(ctx, 0, 6, 0);

  gfx_ctx_issue(ctx);
  mika_yield(1);

  // Wait for the draw to complete (if not yet)
  // before the next round
  gfx_ctx_wait(ctx);
}
```

## Other languages

Support for other languages is work in progress. Planned are:

- Wren (almost done)
- Lua
- Rust
- Pascal
- Many more scripting languages

Take a look at [into the box] if interested in adopting a new language.

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

### System call

At assembly level, a system call is issued by putting the call number at
register R7 and arguments at R0-R3, then invoking a software interrupt #0
(`SWI #0`). The system call follows the AAPCS32 standard; in particular, the
return value, if any, is saved at R0-R1 in little-endian format.

Pointers are expressed as virtual addresses in application space.

For a comprehensive list of system calls see [here].

[here]: #TODO
