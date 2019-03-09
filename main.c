#include <stdio.h>

#include "oil.h"

int main(int argc, char** argv)
{
    int result = oilGetChunks("img.png");

    if(!result) {
        oilPrintError();
        exit(EXIT_FAILURE);
    }

    return 0;
}