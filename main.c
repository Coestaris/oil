//
// Created by maxim on 3/18/19.
//

#include "oil.h"

void drawRectangle1(colorMatrix* matrix)
{
    oilGrSetPixel(matrix, 0, 0, color(0, 0, 0xFF, 0xFF));
    oilGrSetPixel(matrix, 1, 0, color(0, 0xFF, 0, 0xFF));
    oilGrSetPixel(matrix, 0, 1, color(0xFF, 0, 0, 0xFF));
    oilGrSetPixel(matrix, 1, 1, color(0xFF, 0xFF, 0xFF, 0xFF));
}

void drawRectangle(colorMatrix* matrix)
{
    oilGrFill(matrix, color(0xCC, 0xCC, 0xCC, 0xFF));
    oilGrSetPixel(matrix, 1, 1, color(0xFF, 0, 0, 0xFF));
    oilGrSetPixel(matrix, 2, 1, color(0, 0xFF, 0, 0xFF));
    oilGrSetPixel(matrix, 3, 1, color(0, 0, 0xFF, 0xFF));

    oilGrSetPixel(matrix, 3, 3, color(0xFF, 0, 0, 0xFF));
    oilGrSetPixel(matrix, 2, 3, color(0, 0xFF, 0, 0xFF));
    oilGrSetPixel(matrix, 1, 3, color(0, 0, 0xFF, 0xFF));

    oilGrSetPixel(matrix, 1, 2, color(0xFF, 0, 0xFF, 0xFF));
    oilGrSetPixel(matrix, 3, 2, color(0xFF, 0, 0xFF, 0xFF));
}

int main(int argc, char** argv)
{
    bmpImage* image = createBMPImage(2, 2, 24);
    drawRectangle1(image->matrix);
    /*
    for (uint32_t y = 0; y < image->infoHeader->height; y++)
    {
        for(uint32_t x = 0; x < image->infoHeader->width; x++)
        {
            oilColor color = oilGrGetPixel(image->matrix, x, y);
            oilPrintColor(&color, 1);
        }
        putchar('\n');
    }
    */

    if(!oilBMPSave(image, "image.bmp"))
    {
        oilPrintError();
    }

    oilBMPFreeImage(image);
    return 0;
}