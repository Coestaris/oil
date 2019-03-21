//
// Created by maxim on 3/18/19.
//

#ifndef OIL_GRAPHICS_H
#define OIL_GRAPHICS_H

#include <GL/freeglut.h>
#include <GL/gl.h>

#include "oilerror.h"
#include "png.h"
#include "assert.h"

#define OIL_GRAPHICS_CLIP_CHECKING

typedef struct {
    colorMatrix* srcMatrix;
    uint8_t* data;
    size_t dataLen;

    uint32_t componentFormat;
    uint32_t dataFormat;

} imageData;

#define GL_AUTO -1

GLuint oilTextureFromFile(char* filename, uint32_t componentFormat, uint32_t dataFormat);

GLuint oilGetTexture(imageData* img);
void oilFreeImageData(imageData* data);

imageData* oilGetPNGImageData(pngImage *img, uint32_t componentFormat, uint32_t dataFormat);

void oilGrFill(colorMatrix* matrix, oilColor color);
void oilGrSetPixel(colorMatrix* matrix, uint32_t x, uint32_t y, oilColor color);
oilColor oilGrGetPixel(colorMatrix* matrix, uint32_t x, uint32_t y);

#endif //OIL_GRAPHICS_H
