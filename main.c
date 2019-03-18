#include <stdio.h>

#include "oil.h"

int main(int argc, char** argv)
{
    pngImage* image;
    if(!(image = oilLoad("img.png")))
    {
        oilPrintError();
        exit(EXIT_FAILURE);
    }

    for (uint32_t y = 0; y < image->height; y++)
    {
        for(uint32_t x = 0; x < image->width; x++)
        {
            oilPrintColor(image->colors[y][x], image->colorFlag, 0);
            putchar(' ')
        }
        putchar('\n');
    }

    oilFreeImage(image);

    return 0;
}