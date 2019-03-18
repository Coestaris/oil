#include <stdio.h>

#include "oil.h"

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    GLuint tex = oilTextureFromFile("img.png", GL_RGB, GL_UNSIGNED_BYTE);
    if(tex == 0)
    {
        oilPrintError();
        exit(EXIT_FAILURE);
    }
}