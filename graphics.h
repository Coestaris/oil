//
// Created by maxim on 3/18/19.
//

#ifndef OIL_GRAPHICS_H
#define OIL_GRAPHICS_H

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>

#ifdef OIL_USE_GLUT
#include <GL/OIL_FREEglut.h>
#endif

#include <stdlib.h>
#include "oilerror.h"
#include "png.h"
#include "assert.h"
#include "font.h"

#define OIL_GRAPHICS_CLIP_CHECKING

typedef struct _imageData
{
   colorMatrix* srcMatrix;
   uint8_t* data;
   size_t dataLen;

   uint32_t componentFormat;
   uint32_t dataFormat;

} imageData;

#define GL_AUTO -1

#define OIL_DEFAULT_MIN         GL_LINEAR_MIPMAP_LINEAR
#define OIL_DEFAULT_MAG         GL_LINEAR
#define OIL_DEFAULT_DATAFMT     GL_UNSIGNED_BYTE
#define OIL_DEFAULT_WRAPPING    GL_CLAMP_TO_EDGE
#define OIL_DEFAULT_BORDERCOLOR NULL
#define OIL_DEFAULT_FLIPX       0
#define OIL_DEFAULT_FLIPY       0

#define OIL_TEX_MAG             1UL
#define OIL_TEX_MIN             2UL
#define OIL_TEX_DATAFRMT        4UL
#define OIL_TEX_WRAPPING        8UL
#define OIL_TEX_BORDERCOLOR     16UL
#define OIL_TEX_FLIPX           32UL
#define OIL_TEX_FLIPY           64UL

typedef struct _texData
{
   GLenum magFilter;
   GLenum minFilter;
   GLenum dataFormat;
   GLenum wrappingMode;
   float* borderColor;
   uint8_t flipX;
   uint8_t flipY;

   uint32_t out_width;
   uint32_t out_height;

} texData;

GLuint oilTextureFromPngFileDef(char* filename, uint32_t componentFormat);
GLuint oilTextureFromPngFile(char* filename, uint32_t componentFormat, uint32_t flags, texData* data);

GLuint oilGetTexture(imageData* img, GLenum wrapping, GLenum magFilter, GLenum minFilter, float* borderColor);
void oilFreeImageData(imageData* data);

imageData* oilGetPNGImageData(pngImage* img, uint32_t componentFormat, uint32_t dataFormat);

void oilGrFlipX(colorMatrix* matrix);
void oilGrFlipY(colorMatrix* matrix);
void oilGrFill(colorMatrix* matrix, oilColor color);
void oilGrDrawLine(colorMatrix* matrix, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, oilColor color);
void oilGrDrawLineSm(colorMatrix* matrix, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, oilColor color);

void oilGrDrawCircle(colorMatrix* matrix, uint32_t center_x, uint32_t center_y, uint32_t radius, oilColor color);
void oilGrDrawCircleSm(colorMatrix* matrix, uint32_t center_x, uint32_t center_y, uint32_t radius, oilColor color);

void oilGrFillCircle(colorMatrix* matrix, uint32_t center_x, uint32_t center_y, uint32_t radius, oilColor color);
void oilGrDrawString(colorMatrix* matrix, oilFont* font, char* string, uint32_t x, uint32_t y, oilColor color);
void oilGrDrawCenteredString(colorMatrix* matrix, oilFont* font, char* string, uint32_t x, uint32_t y, oilColor color);
void oilGrSetPixel(colorMatrix* matrix, uint32_t x, uint32_t y, oilColor color);
oilColor oilGrGetPixel(colorMatrix* matrix, uint32_t x, uint32_t y);

#pragma clang diagnostic pop
#endif //OIL_GRAPHICS_H

