#include "charbuf.h"
#include "main.h"
#include "kmalloc.h"
#include "utils/font_bitmap.h"
/*
#include "utils/FantasqueSansMono-Regular.ttf.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_assert(x)
#define STBTT_malloc(x, u)  ((void)(u), kmalloc(x))
#define STBTT_free(x, u)    ((void)(u), kfree(x))
#include "stb/stb_truetype.h"
*/

#include <stdint.h>
#include <string.h>

#define MAX_ROWS  32
#define MAX_COLS  64
#define CHAR_W    12
#define CHAR_H    24
#define CHAR_BL   18  // Baseline
static char buf[MAX_ROWS][MAX_COLS];
static uint32_t w, h, rs, cs;
static uint32_t r, c;

/*
static stbtt_fontinfo font;
static uint8_t bitmap[96][CHAR_H][CHAR_W];
*/

void charbuf_init(uint32_t width, uint32_t height)
{
  w = width;
  h = height;
  rs = height / CHAR_H;
  cs = width / CHAR_W;

  r = c = 0;
  memset(buf, 0, sizeof buf);

/*
  stbtt_InitFont(&font, FantasqueSansMono_Regular_ttf,
    stbtt_GetFontOffsetForIndex(FantasqueSansMono_Regular_ttf, 0));

  float scale = stbtt_ScaleForPixelHeight(&font, 24);
  memset(bitmap, 0, sizeof bitmap);

  for (char c = 32; c != (char)128; c++) {
    kmalloc_reset();
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(
      &font, c, scale, scale, &x0, &y0, &x1, &y1);
    stbtt_MakeCodepointBitmap(&font,
      &bitmap[c - 32][0][0] + CHAR_W * (CHAR_BL + y0) + x0, CHAR_W,
      CHAR_H - (CHAR_BL + y0), CHAR_W, scale, scale, c);
  }
*/
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
  uint32_t blend[256];
  for (uint32_t i = 0; i < 256; i++) {
    uint8_t r = (0x2e * (255 - i) + 0xe5 * i) / 255;
    uint8_t g = (0x34 * (255 - i) + 0xe9 * i) / 255;
    uint8_t b = (0x40 * (255 - i) + 0xf0 * i) / 255;
    blend[i] = (r << 16) | (g << 8) | b;
  }

  for (uint32_t r = 0, y0 = 0; r <= rs; r++, y0 += CHAR_H)
  for (uint32_t c = 0, x0 = 0; c <= cs; c++, x0 += CHAR_W)
    for (uint32_t y = 0; y < CHAR_H && y0 + y < h; y++)
    for (uint32_t x = 0; x < CHAR_W && x0 + x < w; x++) {
      uint8_t pix = (buf[r][c] < 32 ? 0 : bitmap[buf[r][c] - 32][y][x]);
      put_pixel(x0 + x, y0 + y, blend[pix]);
    }
}
