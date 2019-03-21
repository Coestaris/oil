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
                matrix->matrix[i][j] = malloc(sizeof(oilColor));
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

oilColor color(uint16_t r, uint16_t g, uint16_t b, uint16_t a)
{
    oilColor c = { r, g, b, a };
    return c;
}