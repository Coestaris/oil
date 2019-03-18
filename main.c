//
// Created by maxim on 3/18/19.
//

#include "oil.h"

int main(int argc, char** argv)
{
    pngImage* image;
    if((image = oilPNGLoad("img.png", 0)) == NULL)
    {
        oilPrintError();
        exit(EXIT_FAILURE);
    }

    oilPNGFreeImage(image);

    return 0;
}