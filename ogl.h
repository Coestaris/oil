//
// Created by maxim on 3/18/19.
//

#ifndef OIL_OGL_H
#define OIL_OGL_H

#include <GL/freeglut.h>
#include <GL/gl.h>

#include "oilerror.h"
#include "png.h"

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

#endif //OIL_OGL_H
