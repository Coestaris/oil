//
// Created by maxim on 3/18/19.
//

#include <stdbool.h>
#include "graphics.h"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"

#ifndef OIL_USE_GLUT

char* gluErrorString(GLenum error)
{
   //stub!
   return "";
}

#endif

void writeComponent(uint8_t* data, size_t* byteCounter, uint32_t dataFormat, uint16_t component)
{
   switch (dataFormat)
   {
      default:
      case GL_UNSIGNED_BYTE:
         data[(*byteCounter)++] = (uint8_t) component;
         break;
      case GL_BYTE:
         data[(*byteCounter)++] = (int8_t) component;
         break;
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_SHORT:
         data[(*byteCounter)++] = (uint8_t) component;
         data[(*byteCounter)++] = (uint8_t) (component << 8U);
         break;
      case GL_SHORT:
      case GL_INT:
         data[(*byteCounter)++] = (int8_t) component;
         data[(*byteCounter)++] = (int8_t) (component << 8U);
         break;
   }
}

#define wrComponent(comp) writeComponent(data, byteCounter, dataFormat, comp)

void writeColor(uint8_t* data, size_t* byteCounter, oilColor* color, uint32_t componentFormat, uint32_t dataFormat)
{
   switch (componentFormat)
   {
      case GL_RED:
         wrComponent(color->r);
         break;

      case GL_RG:
         wrComponent(color->r);
         wrComponent(color->g);
         break;

      case GL_RGB:
         wrComponent(color->r);
         wrComponent(color->g);
         wrComponent(color->b);
         break;

      case GL_BGR:
         wrComponent(color->b);
         wrComponent(color->g);
         wrComponent(color->r);
         break;

      default:
      case GL_RGBA:
         wrComponent(color->r);
         wrComponent(color->g);
         wrComponent(color->b);
         wrComponent(color->a);
         break;

      case GL_BGRA:
         wrComponent(color->b);
         wrComponent(color->g);
         wrComponent(color->r);
         wrComponent(color->a);
         break;
   }
}

imageData* oilGetPNGImageData(pngImage* img, uint32_t componentFormat, uint32_t dataFormat)
{
   /* Allowed componentFormats
    * - GL_RED,
    * - GL_RG,
    * - GL_RGB,
    * - GL_BGR,
    * - GL_RGBA,
    * - GL_BGRA
    * - GL_AUTO
    *
    * Allowed dataFormats
    *  - GL_UNSIGNED_BYTE,
    *  - GL_BYTE,
    *  - GL_UNSIGNED_SHORT,
    *  - GL_SHORT,
    *  - GL_UNSIGNED_INT,
    *  - GL_INT
    *  - GL_AUTO
    */

   if (componentFormat != GL_RED && componentFormat != GL_RG && componentFormat != GL_RGB &&
       componentFormat != GL_BGR && componentFormat != GL_BGRA && componentFormat != GL_RGBA &&
       componentFormat != GL_AUTO)
   {
      oilPushError("[OILERROR]: Unknown component format\n");
      return NULL;
   }

   if (dataFormat != GL_UNSIGNED_BYTE && dataFormat != GL_BYTE && dataFormat != GL_UNSIGNED_SHORT &&
       dataFormat != GL_SHORT && dataFormat != GL_UNSIGNED_INT && dataFormat != GL_INT &&
       dataFormat != GL_AUTO)
   {
      oilPushError("[OILERROR]: Unknown data format\n");
      return NULL;
   }

   imageData* data = malloc(sizeof(imageData));
   data->srcMatrix = img->colorMatrix;
   data->componentFormat = componentFormat;
   data->dataFormat = dataFormat;

   if (componentFormat == GL_AUTO)
   {
      if (!img->pixelsInfo->useColor)
      {
         componentFormat = GL_RED;
      }
      else
      {
         if (img->pixelsInfo->hasAlpha) componentFormat = GL_RGBA;
         else componentFormat = GL_RGB;
      }
   }

   if (dataFormat == GL_AUTO)
   {
      switch (img->pixelsInfo->bitDepth)
      {
         case 1:
         case 2:
         case 4:
         case 8:
            dataFormat = GL_UNSIGNED_BYTE;
            break;
         case 16:
            dataFormat = GL_UNSIGNED_SHORT;
            break;
         default:
            oilPushError("[OILERROR]: When using GL_AUTO bitdepth must be valid value\n");
            free(data);
            return NULL;
      }
   }

   uint8_t bytesPerPixel = 0;
   switch (componentFormat)
   {
      case GL_RED:
         bytesPerPixel = 1;
         break;
      case GL_RG:
         bytesPerPixel = 2;
         break;
      case GL_RGB:
      case GL_BGR:
         bytesPerPixel = 3;
         break;
      case GL_RGBA:
      case GL_BGRA:
         bytesPerPixel = 4;
         break;
      default:
         break;
   }

   switch (dataFormat)
   {
      case GL_UNSIGNED_BYTE:
      case GL_BYTE:
         bytesPerPixel *= 1;
         break;
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
         bytesPerPixel *= 2;
         break;
      case GL_UNSIGNED_INT:
      case GL_INT:
         bytesPerPixel *= 4;
         break;
      default:
         break;
   }

   data->dataLen = bytesPerPixel * img->width * img->height;
   data->data = malloc(sizeof(uint8_t) * data->dataLen);

   size_t byteCounter = 0;
   for (uint32_t y = 0; y < img->height; y++)
   {
      for (uint32_t x = 0; x < img->width; x++)
      {
         writeColor(data->data, &byteCounter, img->colorMatrix->matrix[y][x], componentFormat, dataFormat);
      }
   }

   return data;
}

void oilFreeImageData(imageData* data)
{
   free(data->data);
   free(data);
}

#define checkGLError(str)  if((error = glGetError()) != GL_NO_ERROR) \
            { \
                oilPushErrorf("[OILERROR]: "str". Gl error: %s (errno %i)\n", gluErrorString(error), error);\
                return 0;\
            }

GLuint oilGetTexture(imageData* data, GLenum wrapping, GLenum magFilter, GLenum minFilter, float* borderColor)
{
   glGetError(); //Reset error flag

   GLenum error;
   GLuint id = 0;

   glGenTextures(1, &id);
   if (((error = glGetError()) != GL_NO_ERROR || id == 0))
   {
      oilPushErrorf("[OILERROR]: Unable to generate texture. Gl error: %s (errno %i)\n", gluErrorString(error), error);
      return 0;
   }

   glBindTexture(GL_TEXTURE_2D, id);
   checkGLError("Unable to bind texture")

   glTexImage2D(GL_TEXTURE_2D,
                0,
                data->componentFormat,
                data->srcMatrix->width,
                data->srcMatrix->height,
                0,
                data->componentFormat,
                data->dataFormat,
                data->data);
   checkGLError("Unable to fill texture data")

   if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST ||
       magFilter == GL_LINEAR_MIPMAP_LINEAR ||
       magFilter == GL_LINEAR_MIPMAP_NEAREST ||
       magFilter == GL_NEAREST_MIPMAP_LINEAR ||
       magFilter == GL_NEAREST_MIPMAP_NEAREST)
   {
      glGenerateMipmap(GL_TEXTURE_2D);
      checkGLError("Unable to generate mipmap")
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
   checkGLError("Unable to set tex parameter (wrapping)")
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapping);
   checkGLError("Unable to set tex parameter (wrapping)")

   if (wrapping == GL_CLAMP_TO_BORDER && borderColor)
   {
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
      checkGLError("Unable to set tex parameter (border color)")
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
   checkGLError("Unable to set tex parameter (min filter)")

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
   checkGLError("Unable to set tex parameter (mag filter)")

   glBindTexture(GL_TEXTURE_2D, 0);
   checkGLError("Unable to unbind texture")

   return id;
}

GLuint oilTextureFromPngFileDef(char* fileName, uint32_t componentFormat)
{
   texData data;
   return oilTextureFromPngFile(fileName, componentFormat, 0, &data);
}

GLuint oilTextureFromPngFile(char* filename, uint32_t componentFormat, uint32_t flags, texData* texData)
{
   if (!(flags & OIL_TEX_MIN)) texData->minFilter = OIL_DEFAULT_MIN;
   if (!(flags & OIL_TEX_MAG)) texData->magFilter = OIL_DEFAULT_MAG;
   if (!(flags & OIL_TEX_BORDERCOLOR)) texData->borderColor = OIL_DEFAULT_BORDERCOLOR;
   if (!(flags & OIL_TEX_DATAFRMT)) texData->dataFormat = OIL_DEFAULT_DATAFMT;
   if (!(flags & OIL_TEX_FLIPX)) texData->flipX = OIL_DEFAULT_FLIPX;
   if (!(flags & OIL_TEX_FLIPY)) texData->flipY = OIL_DEFAULT_FLIPY;
   if (!(flags & OIL_TEX_WRAPPING)) texData->wrappingMode = OIL_DEFAULT_WRAPPING;

   pngImage* image;
   if (!(image = oilPNGLoad(filename, 1)))
   {
      oilPushError("[OILERROR]: Unable to load image");
      return 0;
   }

   if (texData->flipX) oilGrFlipX(image->colorMatrix);
   if (texData->flipY) oilGrFlipY(image->colorMatrix);

   /*oilColor c;
   c = oilGrGetPixel(image->colorMatrix, 0, 0);
   oilPrintColor(&c, 0);

   c = oilGrGetPixel(image->colorMatrix, image->width - 1, 0);
   oilPrintColor(&c, 0);

   c = oilGrGetPixel(image->colorMatrix, 0, image->height - 1);
   oilPrintColor(&c, 0);

   c = oilGrGetPixel(image->colorMatrix, image->width - 1, image->height - 1);
   oilPrintColor(&c, 0);*/


   imageData* data = oilGetPNGImageData(image, componentFormat, texData->dataFormat);
   if (data == NULL)
   {
      oilPushError("[OILERROR]: Unable to get image data");
      oilPNGFreeImage(image);
      return 0;
   }

   texData->out_width = image->width;
   texData->out_height = image->height;

   GLuint tex = oilGetTexture(data, texData->wrappingMode, texData->magFilter,
                              texData->minFilter, texData->borderColor);
   if (tex == 0)
   {
      oilPushError("[OILERROR]: Unable to generate texture");
      oilFreeImageData(data);
      oilPNGFreeImage(image);
      return 0;
   }

   oilFreeImageData(data);
   oilPNGFreeImage(image);

   return tex;
}

void oilGrFill(colorMatrix* matrix, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
#endif

   for (uint32_t i = 0; i < matrix->height; i++)
      for (uint32_t j = 0; j < matrix->width; j++)
         *matrix->matrix[i][j] = color;
}

inline void oilGrSetPixel(colorMatrix* matrix, uint32_t x, uint32_t y, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
   assert(x < matrix->width);
   assert(y < matrix->height);
#endif

   *matrix->matrix[y][x] = color;
}

oilColor oilGrGetPixel(colorMatrix* matrix, uint32_t x, uint32_t y)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
   assert(x < matrix->width);
   assert(y < matrix->height);
#endif

   return *matrix->matrix[y][x];
}

void oilGrFlipX(colorMatrix* matrix)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
#endif

   for (uint32_t i = 0; i < matrix->height; i++)
      for (uint32_t j = 0; j < matrix->width / 2; j++)
      {
         oilColor* tmp = matrix->matrix[i][j];
         matrix->matrix[i][j] = matrix->matrix[i][matrix->width - j - 1];
         matrix->matrix[i][matrix->width - j - 1] = tmp;
      }
}

void oilGrFlipY(colorMatrix* matrix)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
#endif

   for (uint32_t i = 0; i < matrix->height / 2; i++)
      for (uint32_t j = 0; j < matrix->width; j++)
      {
         oilColor* tmp = matrix->matrix[i][j];
         matrix->matrix[i][j] = matrix->matrix[matrix->height - i - 1][j];
         matrix->matrix[matrix->height - i - 1][j] = tmp;
      }

}

#define oilGrSetPixelNoBounds(matrix, x, y, color) \
{                                                  \
   if(x < matrix->width && y < matrix->width)      \
      *matrix->matrix[y][x] = color;               \
}

#define oilGrSetAlphaPixelNoBounds(matrix, x, y, a, color)                 \
{                                                                          \
   if(((uint32_t)x) < matrix->width && ((uint32_t)y) < matrix->height)     \
   {                                                                       \
      oilColor old = *matrix->matrix[(uint32_t)(y)][(uint32_t)(x)];        \
                                                                           \
      old.r = (float)color.r * a + (float)old.r * (1 - a);                 \
      old.g = (float)color.g * a + (float)old.g * (1 - a);                 \
      old.b = (float)color.b * a + (float)old.b * (1 - a);                 \
      *matrix->matrix[(uint32_t)(y)][(uint32_t)(x)] = old;                 \
   }                                                                       \
}

void oilGrDrawCircle(colorMatrix* matrix, uint32_t center_x, uint32_t center_y, uint32_t radius, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
   assert(center_x < matrix->width);
   assert(center_y< matrix->height);
#endif

   int32_t x = 0;
   int32_t y = radius;
   int32_t delta = 1 - 2 * radius;
   int32_t error = 0;
   while (y >= 0)
   {
      oilGrSetPixelNoBounds(matrix, (int32_t)center_x + x, (int32_t)center_y + y, color);
      oilGrSetPixelNoBounds(matrix, (int32_t)center_x + x, (int32_t)center_y - y, color);
      oilGrSetPixelNoBounds(matrix, (int32_t)center_x - x, (int32_t)center_y + y, color);
      oilGrSetPixelNoBounds(matrix, (int32_t)center_x - x, (int32_t)center_y - y, color);
      error = 2 * (delta + y) - 1;
      if ((delta < 0) && (error <= 0))
      {
         delta += 2 * ++x + 1;
         continue;
      }
      if ((delta > 0) && (error > 0))
      {
         delta -= 2 * --y + 1;
         continue;
      }
      delta += 2 * (++x - y--);
   }
}

void oilGrDrawHLine(colorMatrix* matrix, uint32_t x1, uint32_t x2, uint32_t y, oilColor color)
{
   for(uint32_t x = x1; x <= x2; x++)
      oilGrSetPixelNoBounds(matrix, x, y, color);
}

void oilGrDrawVLine(colorMatrix* matrix, uint32_t y1, uint32_t y2, uint32_t x, oilColor color)
{
   for(uint32_t y = y1; y <= y2; y++)
      oilGrSetPixelNoBounds(matrix, x, y, color);
}

void oilGrFillCircle(colorMatrix* matrix, uint32_t center_x, uint32_t center_y, uint32_t radius, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
   assert(center_x < matrix->width);
   assert(center_y < matrix->height);
#endif

   int32_t x = 0;
   int32_t y = radius;
   int32_t delta = 1 - 2 * radius;
   int32_t error = 0;
   while (y >= 0)
   {
      oilGrDrawHLine(matrix, (int32_t)center_x - x,  (int32_t)center_x + x, (int32_t)center_y + y, color);
      oilGrDrawHLine(matrix, (int32_t)center_x - x,  (int32_t)center_x + x, (int32_t)center_y - y, color);
      error = 2 * (delta + y) - 1;
      if ((delta < 0) && (error <= 0))
      {
         delta += 2 * ++x + 1;
         continue;
      }
      if ((delta > 0) && (error > 0))
      {
         delta -= 2 * --y + 1;
         continue;
      }
      delta += 2 * (++x - y--);
   }
}

void oilGrDrawCircleSm(colorMatrix* matrix, uint32_t center_x, uint32_t center_y, uint32_t radius, oilColor color)
{

#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
   assert(center_x < matrix->width);
   assert(center_y < matrix->height);
#endif

   float i = 0;
   float j = radius;
   float last_fade_amount = 0;
   float fade_amount = 0;

   while(i < j)
   {
      float height = sqrtf(fmax(radius * radius - i * i, 0));
      fade_amount = ceilf(height) - height;

      if(fade_amount < last_fade_amount)
         j -= 1;
      last_fade_amount = fade_amount;

      float rfade_amount = 1.0f - fade_amount;
      oilGrSetAlphaPixelNoBounds(matrix, i + center_x, j + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, i + center_x, j - 1 +  + center_y, fade_amount, color);

      oilGrSetAlphaPixelNoBounds(matrix, -i + center_x, j + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, -i + center_x, j - 1 +  + center_y, fade_amount, color);

      oilGrSetAlphaPixelNoBounds(matrix, i + center_x, -j + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, i + center_x, -j + 1 +  + center_y, fade_amount, color);

      oilGrSetAlphaPixelNoBounds(matrix, -i + center_x, -j + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, -i + center_x, -j + 1 +  + center_y, fade_amount, color);

      oilGrSetAlphaPixelNoBounds(matrix, j + center_x, i + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, j - 1 + center_x, i + center_y, fade_amount, color);

      oilGrSetAlphaPixelNoBounds(matrix, -j + center_x, i + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, -j + 1 + center_x, i + center_y, fade_amount, color);

      oilGrSetAlphaPixelNoBounds(matrix, j + center_x, -i + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, j - 1 + center_x, -i + center_y, fade_amount, color);

      oilGrSetAlphaPixelNoBounds(matrix, -j + center_x, -i + center_y, rfade_amount, color);
      oilGrSetAlphaPixelNoBounds(matrix, -j + 1 + center_x, -i + center_y, fade_amount, color);
      i += 1;
   }
}

void oilGrDrawLine(colorMatrix* matrix, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
   assert(x1 < matrix->width);
   assert(y1 < matrix->height);
   assert(x2 < matrix->width);
   assert(y2 < matrix->height);
#endif

   if(y1 == y2) {
      uint32_t hi_x = x1 > x2 ? x1 : x2;
      uint32_t lo_x = x1 > x2 ? x2 : x1;
      oilGrDrawHLine(matrix, lo_x, hi_x, y1, color);
      return;
   }

   if(x1 == x2) {
      uint32_t hi_y = y1 > y2 ? y1 : y2;
      uint32_t lo_y = y1 > y2 ? y2 : y1;
      oilGrDrawVLine(matrix, lo_y, hi_y, x1, color);
      return;
   }

   int32_t dx =  abs((int32_t)x2 - (int32_t)x1);
   int32_t sx = x1 < x2 ? 1 : -1;
   int32_t dy = -abs((int32_t)y2 - (int32_t)y1);
   int32_t sy = y1 < y2 ? 1 : -1;
   int32_t err = dx + dy;


   while (true)
   {
      oilGrSetPixelNoBounds(matrix, x1, y1, color);

      if (x1 == x2 && y1 == y2) break;
      int32_t e2 = err << 1; //*2
      if (e2 >= dy)
      {
         err += dy;
         x1 += sx;
      }
      if (e2 <= dx)
      {
         err += dx;
         y1 += sy;
      }
   }
}

float oilGrIntPart(float f)
{
   return (float)(int32_t)f;
}

float oilGrFracPart(float f)
{
   return (float)f - (float)oilGrIntPart(f);
}

void oilSwap(uint32_t* p1, uint32_t* p2)
{
   uint32_t b = *p1;
   *p1 = *p2;
   *p2 = b;
}

void oilGrDrawLineSm(colorMatrix* matrix, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
   assert(matrix != NULL);
   assert(x1 < matrix->width);
   assert(y1 < matrix->height);
   assert(x2 < matrix->width);
   assert(y2 < matrix->height);
#endif

   if(y1 == y2) {
      uint32_t hi_x = x1 > x2 ? x1 : x2;
      uint32_t lo_x = x1 > x2 ? x2 : x1;
      oilGrDrawHLine(matrix, lo_x, hi_x, y1, color);
      return;
   }

   if(x1 == x2) {
      uint32_t hi_y = y1 > y2 ? y1 : y2;
      uint32_t lo_y = y1 > y2 ? y2 : y1;
      oilGrDrawVLine(matrix, lo_y, hi_y, x1, color);
      return;
   }

   float dx = (float)x2 - (float)x1;
   float dy = (float)y2 - (float)y1;
   if (fabsf(dx) > fabsf(dy))
   {
      if (x2 < x1)
      {
         oilSwap(&x1, &x2);
         oilSwap(&y1, &y2);
      }

      float gradient = dy / dx;
      float xend = oilGrIntPart(x1 + 0.5f);
      float yend = y1 + gradient * (xend - x1);
      float xgap = 1.0f - oilGrFracPart(x1 + 0.5f);
      int32_t xpxl1 = xend;
      int32_t ypxl1 = oilGrIntPart(yend);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl1, ypxl1, (1.0f - oilGrFracPart(yend)) * xgap, color);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl1, ypxl1 + 1, oilGrFracPart(yend) * xgap, color);
      float intery = yend + gradient;

      xend = oilGrIntPart(x2 + 0.5f);
      yend = y2 + gradient * (xend - x2);
      xgap = oilGrFracPart(x2 + 0.5f);
      int32_t xpxl2 = xend;
      int32_t ypxl2 = oilGrIntPart(yend);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl2, ypxl2, (1.0f - oilGrFracPart(yend)) * xgap, color);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl2, ypxl2 + 1, oilGrFracPart(yend) * xgap, color);

      int32_t x;
      for(x=xpxl1+1; x < xpxl2; x++)
      {
         oilGrSetAlphaPixelNoBounds(matrix, x, oilGrIntPart(intery), (1.0f - oilGrFracPart(intery)), color);
         oilGrSetAlphaPixelNoBounds(matrix, x, oilGrIntPart(intery) + 1, oilGrFracPart(intery), color);
         intery += gradient;
      }
   }
   else
   {
      if (y2 < y1)
      {
         oilSwap(&x1, &x2);
         oilSwap(&y1, &y2);
      }
      float gradient = dx / dy;
      float yend = oilGrIntPart(y1 + 0.5f);
      float xend = x1 + gradient*(yend - y1);
      float ygap = 1.0f - oilGrFracPart(y1 + 0.5f);
      int32_t ypxl1 = yend;
      int32_t xpxl1 = oilGrIntPart(xend);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl1, ypxl1, (1.0f - oilGrFracPart(xend)) * ygap, color);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl1 + 1, ypxl1, oilGrFracPart(xend) * ygap, color);
      float interx = xend + gradient;

      yend = oilGrIntPart(y2 + 0.5f);
      xend = x2 + gradient*(yend - y2);
      ygap = oilGrFracPart(y2 + 0.5f);
      int32_t ypxl2 = yend;
      int32_t xpxl2 = oilGrIntPart(xend);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl2, ypxl2, (1.0f - oilGrFracPart(xend)) * ygap, color);
      oilGrSetAlphaPixelNoBounds(matrix, xpxl2 + 1, ypxl2, oilGrFracPart(xend) * ygap, color);

      int32_t y;
      for(y = ypxl1 + 1; y < ypxl2; y++)
      {
         oilGrSetAlphaPixelNoBounds(matrix, oilGrIntPart(interx), y, (1.0f - oilGrFracPart(interx)), color);
         oilGrSetAlphaPixelNoBounds(matrix, oilGrIntPart(interx) + 1, y, oilGrFracPart(interx), color);
         interx += gradient;
      }
   }
}

void oilGrDrawString(colorMatrix* matrix, oilFont* font, char* string, uint32_t x, uint32_t y, oilColor color)
{
   for(size_t i = 0; i < strlen(string); i++)
   {
      oilFontChar* ch = &font->fontChars[string[i]];

      float xpos = x + ch->bearing_x;
      float ypos = y - 2 * ch->height + ch->bearing_y;

      //copy pixels
      for(uint32_t px = 0; px < ch->width; px++)
      for(uint32_t py = 0; py < ch->height; py++)
      {
         uint32_t newX = xpos + px;
         uint32_t newY = ypos + py;

         if(newX > matrix->width ||
            newY > matrix->height)
            continue;

         float new = ((float)(ch->data[py * (uint32_t)ch->width + px]) / 255.0f);
         oilGrSetAlphaPixelNoBounds(matrix, newX, newY, new, color);
      }

      x += ch->advance;
   }
}

void oilGrDrawCenteredString(colorMatrix* matrix, oilFont* font, char* string, uint32_t x, uint32_t y, oilColor color)
{
   uint32_t stringWidth = 0;
   uint32_t stringHeight = 0;
   for(size_t i = 0; i < strlen(string); i++)
   {
      oilFontChar* ch = &font->fontChars[string[i]];
      stringWidth += ch->advance;
      stringHeight = stringWidth > ch->width ? stringHeight : ch->width;
   }

   oilGrDrawString(
         matrix,
         font,
         string,
         x - stringWidth / 2,
         y + stringHeight / 2,
         color);
}

#pragma clang diagnostic pop