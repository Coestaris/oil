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
    uint8_t* data;
    size_t dataLen;

    uint32_t componentFormat;
    uint32_t dataFormat;

} pngImageData;

#define GL_AUTO -1

GLuint oilGetTexture(pngImageData* img);
pngImageData* oilGetImageData(pngImage* img, uint32_t componentFormat, uint32_t dataFormat);
void oilFreeImageData(pngImageData* data);

#endif //OIL_OGL_H
