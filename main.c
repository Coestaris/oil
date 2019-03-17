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

    return 0;
}