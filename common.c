//
// Created by maxim on 3/18/19.
//

#include "common.h"

colorMatrix* oilColorMatrixAlloc(uint8_t allocColors, uint32_t width, uint32_t height)
{
    colorMatrix* matrix = malloc(sizeof(colorMatrix));
    matrix->matrix = malloc(sizeof(oilColor**) * height);

    for(uint32_t i = 0; i < height; i++)
    {
        matrix->matrix[i] = malloc((sizeof(oilColor*) * width));
        if(allocColors)
        {
            for (uint32_t j = 0; j < width; j++)
            {
                matrix->matrix[i][j] = ocolorp(0, 0, 0, 0);
            }
        }
    }
    matrix->width = width;
    matrix->height = height;

    return matrix;
}

void oilColorMatrixFree(colorMatrix* matrix)
{
    for(uint32_t i = 0; i < matrix->height; i++)
    {
        for(uint32_t j = 0; j < matrix->height; j++)
            free(matrix->matrix[i][j]);
        free(matrix->matrix[i]);
    }
    free(matrix->matrix);
    free(matrix);
}


void oilPrintColor(oilColor *color, uint8_t hex)
{
    if(hex)
        printf("RGBA(%.2X, %.2X, %.2X, %.2X)", color->r, color->g, color->b, color->a);
    else
        printf("RGBA(%.3i, %.3i, %.3i, %.3i)", color->r, color->g, color->b, color->a);
}

oilColor ocolor(uint16_t r, uint16_t g, uint16_t b, uint16_t a)
{
    oilColor c = { r, g, b, a };
    return c;
}

oilColor* ocolorp(uint16_t r, uint16_t g, uint16_t b, uint16_t a)
{
    oilColor* c = malloc(sizeof(oilColor));
    c->r = r; c->g = g; c->b = b; c->a = a;
    return  c;
}

colorMatrix* oilColorMatrixCopy(colorMatrix* src, colorMatrix* dest)
{
    if(!dest)
    {
        dest = oilColorMatrixAlloc(1, src->width, src->height);
    } else
    {
        if(dest->height != src->height || dest->width != src->width) {
            oilPushError("Sizes of color matrices should be equal");
            return NULL;
        }
    }

    /*memcpy(dest->matrix, dest->matrix,
           sizeof(oilColor**) * sizeof(oilColor*) * dest->width * dest->height);*/

    for(uint32_t x = 0; x < dest->width; x++)
        for(uint32_t y = 0; y < dest->height; y++)
        {
            dest->matrix[y][x]->r = src->matrix[y][x]->r;
            dest->matrix[y][x]->g = src->matrix[y][x]->g;
            dest->matrix[y][x]->b = src->matrix[y][x]->b;
            dest->matrix[y][x]->a = src->matrix[y][x]->a;
        }
    return dest;
}