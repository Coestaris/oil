//
// Created by maxim on 3/18/19.
//

#ifndef OIL_COMMON_H
#define OIL_COMMON_H

#include <stdint.h>
#include <malloc.h>
#include <string.h>

typedef struct
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t a;

} oilColor;

typedef struct {
    oilColor*** matrix;
    uint32_t width;
    uint32_t height;

} colorMatrix;

colorMatrix* oilColorMatrixAlloc(uint8_t allocColors, uint32_t width, uint32_t height);
void oilColorMatrixFree(colorMatrix* matrix);
void oilPrintColor(oilColor *color, uint8_t hex);
colorMatrix* oilColorMatrixCopy(colorMatrix* src, colorMatrix* dest);

oilColor color(uint16_t r, uint16_t g, uint16_t b, uint16_t a);

#endif //OIL_COMMON_H
