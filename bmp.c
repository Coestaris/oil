//
// Created by maxim on 3/18/19.
//

#include "bmp.h"

bmpImage* allocImage()
{
    bmpImage* image = malloc(sizeof(bmpImage));
    image->fileHeader = malloc(sizeof(bmpFileHeader));
    image->fileHeader->reserved1 = 0;
    image->fileHeader->reserved2 = 0;
    image->infoHeader = malloc(sizeof(bmpInfoHeader));
    image->infoHeader->headerSize = bmpInfoHeaderSize;
    image->colorMatrix = NULL;
    return image;
}

bmpImage* oilBMPLoad(char* fileName)
{
    FILE *f = fopen(fileName, "rb");

    if (!f)
    {
        oilPushErrorf("[OILERROR]: Unable to open file \"%s\"\n", fileName);
        return 0;
    }


}

void oilBMPFreeImage(bmpImage* image)
{
    if(image->colorMatrix) oilColorMatrixFree(image->colorMatrix);
    free(image->infoHeader);
    free(image->fileHeader);
    free(image);
}

bmpImage* createBMPImage(uint32_t width, uint32_t height, uint16_t bitDepth)
{
    bmpImage* image = allocImage();
    image->fileHeader->signature = bmp_signature_bm;

    image->fileHeader->offsetBytes = bmpFileHeaderSize + bmpInfoHeaderSize;

    int toPad = (4 - (width * 3) % 4) % 4;
    image->infoHeader->imageSize = (width * height) * 3 * bitDepth / 24 +
            (width)* toPad;

    image->fileHeader->fileSize =
            image->fileHeader->offsetBytes  +
            image->infoHeader->imageSize;

    image->infoHeader->bitDepth = bitDepth;
    image->infoHeader->compression = bmp_compression_rgb;
    image->infoHeader->colorsImportant = 0;
    image->infoHeader->planes = 1;
    image->infoHeader->XpelsPerMeter = 2835;
    image->infoHeader->YpelsPerMeter = 2835;

    image->infoHeader->colorsImportant = 0;
    image->infoHeader->colorsUsed = 0;

    image->width = width;
    image->height = height;

    image->colorMatrix = oilColorMatrixAlloc(1, width, height);

    return image;
}

#define writeData(var, str) if(fwrite(&var, sizeof(var), 1, f) != 1) { oilPushError("Unable to write "str); fclose(f); return 0; }

uint8_t oilBMPSave(bmpImage* image, char* fileName)
{
    FILE* f = fopen(fileName, "w");

    if(!f) {
        oilPushErrorf("Unable to open file %s", fileName);
        return 0;
    }

    writeData(image->fileHeader->signature,   "fileHeader: signature");
    writeData(image->fileHeader->fileSize,    "fileHeader: fileSize");
    writeData(image->fileHeader->reserved1,   "fileHeader: reserved1");
    writeData(image->fileHeader->reserved2,   "fileHeader: reserved2");
    writeData(image->fileHeader->offsetBytes, "fileHeader: offsetBytes");

    writeData(image->infoHeader->headerSize,      "infoHeader : headerSize");
    writeData(image->width,                       "infoHeader : width");
    writeData(image->height,                      "infoHeader : height");
    writeData(image->infoHeader->planes,          "infoHeader : planes");
    writeData(image->infoHeader->bitDepth,        "infoHeader : bitDepth");
    writeData(image->infoHeader->compression,     "infoHeader : compression");
    writeData(image->infoHeader->imageSize,       "infoHeader : imageSize");
    writeData(image->infoHeader->XpelsPerMeter,   "infoHeader : XpelsPerMeter");
    writeData(image->infoHeader->YpelsPerMeter,   "infoHeader : YpelsPerMeter");
    writeData(image->infoHeader->colorsUsed,      "infoHeader : colorsUsed");
    writeData(image->infoHeader->colorsImportant, "infoHeader : colorsImportant");

    int toPad = (4 - (image->width * 3) % 4) % 4;
    for(uint32_t y = 0; y < image->width; y++)
    {
        for (uint32_t x = 0; x < image->width; x++)
        {
            switch (image->infoHeader->bitDepth)
            {
                case 16:
                    writeData(image->colorMatrix->matrix[y][x]->r, "colorComponent: r");
                    writeData(image->colorMatrix->matrix[y][x]->g, "colorComponent: g");
                    writeData(image->colorMatrix->matrix[y][x]->b, "colorComponent: b");
                    break;

                case 8:
                default:
                {
                    uint8_t r = (uint8_t) image->colorMatrix->matrix[y][x]->r;
                    uint8_t g = (uint8_t) image->colorMatrix->matrix[y][x]->g;
                    uint8_t b = (uint8_t) image->colorMatrix->matrix[y][x]->b;

                    writeData(r, "colorComponent: r");
                    writeData(g, "colorComponent: g");
                    writeData(b, "colorComponent: b");

                    break;
                }
            }

        }

        uint8_t padding = 0;
        for (int i = 0; i < toPad; i++)
            writeData(padding, "colorComponent: padding");
    }

    if(fclose(f) == -1)
    {
        oilPushError("Unable to close file");
        return 0;
    }

    return 1;
}