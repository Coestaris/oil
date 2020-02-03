//
// Created by maxim on 2/3/20.
//

#ifndef OIL_FONT_H
#define OIL_FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct _oilFontChar
{
   uint8_t* data;
   float width;
   float height;
   float bearing_x;
   float bearing_y;
   float advance;

} oilFontChar;

typedef struct _oilFont
{
   oilFontChar fontChars[256];
   uint8_t startIndex;
   uint8_t endIndex;

} oilFont;

bool oilFontInit(void);
bool oilFontFin(void);
oilFont* oilFontLoad(const char* fileName, uint32_t penSize, uint8_t startIndex, uint8_t endIndex);
void oilFontFree(oilFont* font);

#endif //OIL_FONT_H
