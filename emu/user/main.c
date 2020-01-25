#include "mikabox.h"

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

uint8_t qwq[1024];
uint8_t qvq[] = "=~=";
uint8_t quq = { 0 };

__attribute__ ((noinline)) void crt_init()
{
  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
}

void draw()
{
  // Create context
  int ctx = syscall(256 + 0);

  // Create vertex arrays, uniform arrays and shader
  int va1, ua1, sh1;
  va1 = syscall(256 + 32, 6, 3);
  ua1 = syscall(256 + 48, 0);
  sh1 = syscall(256 + 64, "#C");
  int va2, ua2, sh2;
  va2 = syscall(256 + 32, 6, 4);
  ua2 = syscall(256 + 48, 0);
  sh2 = syscall(256 + 64, "#CA");
  int va3, ua3, sh3, sh3t, tex3;
  va3 = syscall(256 + 32, 6, 6);
  ua3 = syscall(256 + 48, 2);
  sh3 = syscall(256 + 64, "#TCA");
  sh3t = syscall(256 + 64, "#T");
#define tw 96
#define th 64
  tex3 = syscall(256 + 16, tw, th);

  int ia = syscall(256 + 96, 3);

  // Create batch
  int bat1, bat2, bat3, bat3t;
  bat1 = syscall(256 + 80, va1, ua1, sh1);
  bat2 = syscall(256 + 80, va2, ua2, sh2);

  // Close and reopen
  for (int i = 0; i < 20; i++) {
    syscall(256 + 47, va2);
    syscall(256 + 63, ua2);
    syscall(256 + 79, sh2);
    syscall(256 + 111, ia);
    syscall(256 + 95, bat1);
    syscall(256 + 95, bat2);
    va2 = syscall(256 + 32, 6, 4);
    ua2 = syscall(256 + 48, 0);
    sh2 = syscall(256 + 64, "#CA");
    ia = syscall(256 + 96, 3);
    bat1 = syscall(256 + 80, va1, ua1, sh1);
    bat2 = syscall(256 + 80, va2, ua2, sh2);
  }

  // Set up texture and batch 3
  static uint8_t z[th][tw][4];
  for (int i = 0; i < th; i++)
    for (int j = 0; j < tw; j++) {
      z[i][j][0] = 0xff;
      z[i][j][1] = 0xff - i;
      z[i][j][2] = 0xff - j;
      z[i][j][3] = 0xdd;
    }
  syscall(256 + 17, tex3, z, 0);
  syscall(256 + 50, ua3, 0, tex3, (2 << 0) | (2 << 2));
  bat3 = syscall(256 + 80, va3, ua3, sh3);
  bat3t = syscall(256 + 80, va3, ua3, sh3t);

  // Populate index buffer
  uint16_t idxs[3] = {0, 1, 2};
  syscall(256 + 97, ia, 0, idxs, 3);

  float attr[8];
  attr[3] = 1.0;
  attr[4] = 0.8;
  for (int i = 0; i <= 1; i++) {
    attr[0] = attr[1] = (i == 0 ? -0.5 : +0.5);
    attr[2] = 0.9;
    syscall(256 + 33, va1, i * 3 + 0, &attr[0], 1);
    attr[0] = 0.5; attr[1] = -0.5;
    attr[2] = 0.7;
    syscall(256 + 33, va1, i * 3 + 1, &attr[0], 1);
    attr[0] = -0.5; attr[1] = 0.5;
    attr[2] = 0.5;
    syscall(256 + 33, va1, i * 3 + 2, &attr[0], 1);
  }
  attr[2] = 1.0;
  attr[3] = 0.5;
  for (int i = 0; i <= 1; i++) {
    attr[0] = attr[1] = (i == 0 ? -0.5 : +0.5);
    attr[4] = 0.9; attr[5] = 1.0;
    syscall(256 + 33, va2, i * 3 + 0, &attr[0], 1);
    attr[0] = 0.8; attr[1] = -0.2;
    attr[4] = 0.7; attr[5] = 0.3;
    syscall(256 + 33, va2, i * 3 + 1, &attr[0], 1);
    attr[0] = -0.2; attr[1] = 0.8;
    attr[4] = 0.5; attr[5] = 0.3;
    syscall(256 + 33, va2, i * 3 + 2, &attr[0], 1);
  }

  attr[5] = 1.0;
  attr[6] = 1.0;
  for (int i = 0; i <= 1; i++) {
    attr[0] = attr[1] = (i == 0 ? 0.6 : 0.9);
    attr[2] = attr[3] = (i == 0 ? -1 : +2);
    attr[4] = 1.0; attr[7] = 1.0;
    syscall(256 + 33, va3, i * 3 + 0, &attr[0], 1);
    attr[0] = 0.6; attr[1] = 0.9;
    attr[2] = -1; attr[3] = 2;
    syscall(256 + 33, va3, i * 3 + 1, &attr[0], 1);
    attr[0] = 0.9; attr[1] = 0.6;
    attr[2] = 2; attr[3] = -1;
    attr[4] = 0.5; attr[7] = 0.1;
    syscall(256 + 33, va3, i * 3 + 2, &attr[0], 1);
  }

  while (1) {
    // Wait
    syscall(256 + 5, ctx);

    // Reset
    uint64_t t = syscall64(4, 0);
    syscall(256 + 1, ctx, syscall(256 + 18),
      t == 0 ? 0xffffffff : 0xffffddcc);

    // Use batch 1 and add call
    syscall(256 + 2, ctx, bat1);
    syscall(256 + 3, ctx, 0, 3, t == 0 ? 0 : 3);

    // Use batch 2 and add call
    syscall(256 + 2, ctx, bat2);
    idxs[0] = (t == 0 ? 1 : 3);
    syscall(256 + 97, ia, 1, idxs, 1);
    syscall(256 + 3, ctx, 1, 3, ia);

    // Use batch 3 and add call
    uint64_t tick = syscall64(2);
    syscall(256 + 2, ctx, tick % 1000000 < 500000 ? bat3 : bat3t);
    syscall(256 + 3, ctx, 0, 6, 0);

    // Issue and wait
    syscall(256 + 4, ctx);
    syscall(1);
  }
}

void synth()
{
  while (1) {
    syscall(1);
  }
}

void update()
{
  char s[17] = { 0 };
  while (1) {
    syscall(1);
    uint64_t t = syscall64(4, 0);
    for (int j = 15; j >= 0; j--) {
      s[j] = "0123456789abcdef"[t & 0xf];
      t >>= 4;
    }
    syscall(7, s);
  }
}

void main()
{
  crt_init();

  syscall(7, "Hello world!\n");
  syscall(0, update, synth, draw);
  syscall(1);
}
