#define STB_TRUETYPE_IMPLEMENTATION
#include "../stb/stb_truetype.h"

#include <stdint.h>
#include <stdio.h>

int main()
{
  FILE *fp = fopen("FantasqueSansMono-Regular.ttf", "rb");
  uint8_t fbuf[262144];
  fread(fbuf, 1, sizeof fbuf, fp);
  fclose(fp);

  stbtt_fontinfo font;

  stbtt_InitFont(&font, fbuf,
    stbtt_GetFontOffsetForIndex(fbuf, 0));

  // Character width, height, baseline
  int w = 10, h = 20, bl = 15;
  uint8_t *bitmap = (uint8_t *)malloc(w * (h + 1));

  fp = fopen("font_bitmap.h", "w");

  for (char c = 32; c != (char)128; c++) {
    int x0, y0, x1, y1;
    float scale = stbtt_ScaleForPixelHeight(&font, h);

    stbtt_GetCodepointBitmapBox(
      &font, c, scale, scale, &x0, &y0, &x1, &y1);

    memset(bitmap, 0, w * h);
    stbtt_MakeCodepointBitmap(&font,
      bitmap + w * (bl + y0) + x0, w, h - (bl + y0), w, scale, scale, c);

    if (c == 32)
      fprintf(fp, "uint8_t bitmap[%d][%d][%d] = {\n{\n", 128 - 32, h, w);
    else fprintf(fp, "},{\n");

    for (int i = 0; i < h; i++) {
      fprintf(fp, "  {");
      for (int j = 0; j < w; j++)
        fprintf(fp, "%3d%c", bitmap[i * w + j], j == w - 1 ? '}' : ',');
      fprintf(fp, ",\n");
    }
  }

  fprintf(fp, "}\n};\n");
  fclose(fp);

  return 0;
}
