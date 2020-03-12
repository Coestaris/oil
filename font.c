//
// Created by maxim on 2/3/20.
//
#include "font.h"
#include "oilerror.h"
#include "coredef.h"

const char* FT_Error_String(FT_Error err) {
   #undef __FTERRORS_H__
   #define FT_ERRORDEF( e, v, s )  case e: return s;
   #define FT_ERROR_START_LIST     switch (err) {
   #define FT_ERROR_END_LIST       }
   #include FT_ERRORS_H
   return "[unknown error]";
}

FT_Library ftLibrary;

bool oilFontInit(void)
{
   FT_Error errorCode;
   if ((errorCode = FT_Init_FreeType(&ftLibrary)))
   {
      oilPushErrorf("Couldn't init FreeType Library. Error code: %i", errorCode);
      return false;
   }
   return true;
}

void oilFontFree(oilFont* font)
{
   for(uint8_t c = font->startIndex; c < font->endIndex; c++)
      OIL_FREE(font->fontChars[c].data);
   OIL_FREE(font);
}

oilFont* oilFontLoad(const char* fileName, uint32_t penSize, uint8_t startIndex, uint8_t endIndex)
{
   FT_Face face;
   FT_Error errorCode;

   FILE* f = fopen(fileName, "r");
   if (!f)
   {
      oilPushErrorf("Unable to open file %s", fileName);
      return NULL;
   }
   fclose(f);

   if ((errorCode = FT_New_Face(ftLibrary, fileName, 0, &face)))
   {
      oilPushErrorf("Failed to load font. Error: \"%s\" (Error code %i)", FT_Error_String(errorCode), errorCode);
      return NULL;
   }

   if ((errorCode = FT_Set_Pixel_Sizes(face, penSize, 0)))
   {
      oilPushErrorf("Failed set pixel sizes. Error: \"%s\" (Error code %i)", FT_Error_String(errorCode), errorCode);
      return NULL;
   }

   oilFont* font = OIL_MALLOC(sizeof(oilFont));
   memset(font->fontChars, 0, sizeof(font->fontChars));
   font->endIndex = endIndex;
   font->startIndex = startIndex;
   for(uint8_t c = startIndex; c < endIndex; c++)
   {
      if ((errorCode = FT_Load_Char(face, c, FT_LOAD_RENDER)))
      {
         oilPushErrorf("Failed to load %i Glyph. Error: \"%s\" (Error code %i)", FT_Error_String(errorCode), errorCode);
         return NULL;
      }

      size_t len = face->glyph->bitmap.width * face->glyph->bitmap.rows * sizeof(uint8_t);
      font->fontChars[c].data = OIL_MALLOC(len);
      memcpy(font->fontChars[c].data, face->glyph->bitmap.buffer, len);

      font->fontChars[c].width = face->glyph->bitmap.width;
      font->fontChars[c].height = face->glyph->bitmap.rows;
      font->fontChars[c].bearing_x = face->glyph->bitmap_left;
      font->fontChars[c].bearing_y = face->glyph->bitmap_top;
      font->fontChars[c].advance = face->glyph->advance.x >> 6;
   }

   if((errorCode = FT_Done_Face(face)))
   {
      oilPushErrorf("Couldn't free face. Error: \"%s\" (Error code %i)", FT_Error_String(errorCode), errorCode);
      return NULL;
   }
   return font;
}

bool oilFontFin(void)
{
   FT_Error errorCode;
   if((errorCode = FT_Done_FreeType(ftLibrary)))
   {
      oilPushErrorf("Couldn't free library. Error: \"%s\" (Error code %i)", FT_Error_String(errorCode), errorCode);
      return false;
   }
   return true;
}