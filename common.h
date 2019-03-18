//
// Created by maxim on 3/18/19.
//

#ifndef OIL_COMMON_H
#define OIL_COMMON_H

#include <stdint.h>
#include <malloc.h>

typedef struct
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint8_t a;

} pngColor;

typedef struct {
    pngColor*** matrix;
    uint32_t width;
    uint32_t height;

} colorMatrix;

colorMatrix* oilColorMatrixAlloc(uint8_t allocColors, uint32_t width, uint32_t height);
void oilColorMatrixFree(colorMatrix* matrix);
void oilPrintColor(pngColor *color, uint8_t hex);

#endif //OIL_COMMON_H
