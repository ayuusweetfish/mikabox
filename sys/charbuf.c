#include "charbuf.h"
#include "main.h"

#include <stdint.h>
#include <string.h>

#define MAX_ROWS  32
#define MAX_COLS  64
#define CHAR_W    12
#define CHAR_H    24
static char buf[MAX_ROWS][MAX_COLS];
static uint32_t w, h, rs, cs;
static uint32_t r, c;

void charbuf_init(uint32_t width, uint32_t height)
{
  w = width;
  h = height;
  rs = height / CHAR_H;
  cs = width / CHAR_W;
  r = c = 0;
  memset(buf, 0, sizeof buf);
}

void _putchar(char character)
{
  buf[r][c] = character;
  if (++c == cs) {
    c = 0;
    if (++r == rs) r = 0;
    memset(buf[r], 0, cs);
  }
}

void charbuf_flush()
{
  for (uint32_t r = 0, y0 = 0; r <= rs; r++, y0 += CHAR_H)
  for (uint32_t c = 0, x0 = 0; c <= cs; c++, x0 += CHAR_W)
    for (uint32_t y = 0; y < CHAR_H && y0 + y < h; y++)
    for (uint32_t x = 0; x < CHAR_W && x0 + x < w; x++)
      put_pixel(x0 + x, y0 + y,
        buf[r][c] == 0 ? 0x2e3440 : 0xe5e9f0);
}
