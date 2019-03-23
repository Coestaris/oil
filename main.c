//
// Created by maxim on 3/18/19.
//

#include "oil.h"

int main(int argc, char** argv)
{
    pngImage* src;
    if(!(src = oilPNGLoad("sample2.png", 1)))
    {
        oilPrintError();
        exit(EXIT_FAILURE);
    }

    bmpImage* dest = createBMPImage(src->width, src->height, 24);
    oilGrFill(dest->colorMatrix, color(0, 0, 0, 0));
    oilColorMatrixCopy(src->colorMatrix, dest->colorMatrix);

    //dest->colorMatrix = dest->colorMatrix;


    for(uint32_t y = 0; y < src->height; y++) {
        for(uint32_t x = 0; x < src->width; x++){
            oilColor color = oilGrGetPixel(src->colorMatrix, x, y);
            oilPrintColor(&color, 1);
        }
        putchar('\n');
    }

    if(!oilBMPSave(dest, "image.bmp"))
    {
        oilPrintError();
    }
    putchar('\n');

    for(uint32_t y = 0; y < src->height; y++) {
        for(uint32_t x = 0; x < src->width; x++){
            oilColor color = oilGrGetPixel(dest->colorMatrix, x, y);
            oilPrintColor(&color, 1);
        }
        putchar('\n');
    }

    oilPNGFreeImage(src);
    oilBMPFreeImage(dest);
    return 0;
}