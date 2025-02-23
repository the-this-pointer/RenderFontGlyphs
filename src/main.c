/* Created By: Justin Meiners (2013) */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../inc/reshaper.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h" /* http://nothings.org/stb/stb_image_write.h */

#define STB_TRUETYPE_IMPLEMENTATION
#include "../stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

void readFont(const char *path, void **data) {
  FILE *fontFile = fopen(path, "rb");
  fseek(fontFile, 0, SEEK_END);
  long size = ftell(fontFile); /* how long is the file ? */
  fseek(fontFile, 0, SEEK_SET); /* reset */

  *data = malloc(size);
  fread(*data, size, 1, fontFile);
  fclose(fontFile);
}

int main(int argc, const char *argv[]) {
  /* load font file */
  unsigned char *fontBuffer;
  readFont("font/Vazirmatn-Regular.ttf", (void **) &fontBuffer);

  /* prepare font */
  stbtt_fontinfo info;
  if (!stbtt_InitFont(&info, fontBuffer, 0)) {
    printf("failed\n");
  }

  int b_w = 512; /* bitmap width */
  int b_h = 128; /* bitmap height */
  int l_h = 64; /* line height */

  /* create a bitmap for the phrase */
  unsigned char *bitmap = calloc(b_w * b_h, sizeof(unsigned char));

  /* calculate font scaling */
  float scale = stbtt_ScaleForPixelHeight(&info, l_h);

  int wordLen = 9;
  uint16_t arabicWord[9] = {0x0633, 0x0644, 0x0627, 0x0645, ' ', 0x062F, 0x0646, 0x06CC, 0x0627};
  uint16_t word[9] = {0x00};
  convertArabic(arabicWord, wordLen, word);

  int x = 0;

  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

  ascent = roundf(ascent * scale);
  descent = roundf(descent * scale);

  int i;
  for (i = wordLen-1; i >= 0; --i) {
    /* how wide is this character */
    int ax;
    int lsb;
    stbtt_GetCodepointHMetrics(&info, word[i], &ax, &lsb);
    /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) */

    /* get bounding box for character (may be offset to account for chars that dip above or below the line) */
    int c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

    /* compute y (different characters have different heights) */
    int y = ascent + c_y1;

    /* render character (stride and offset is important here) */
    int byteOffset = x + roundf(lsb * scale) + (y * b_w);
    stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, word[i]);

    /* advance x */
    x += roundf(ax * scale);

    /* add kerning */
    int kern;
    kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
    x += roundf(kern * scale);
  }

  /* save out a 1 channel image */
  stbi_write_png("out.png", b_w, b_h, 1, bitmap, b_w);

  /*
   Note that this example writes each character directly into the target image buffer.
   The "right thing" to do for fonts that have overlapping characters is
   MakeCodepointBitmap to a temporary buffer and then alpha blend that onto the target image.
   See the stb_truetype.h header for more info.
  */

  free(fontBuffer);
  free(bitmap);

  return 0;
}

