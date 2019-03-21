//
// Created by maxim on 3/18/19.
//

#include "bmp.h"

bmpImage* allocImage()
{
    bmpImage* image = malloc(sizeof(bmpImage));
    image->fileHeader = malloc(sizeof(bmpFileHeader));
    image->infoHeader = malloc(sizeof(bmpInfoHeader));
    image->infoHeader->headerSize = sizeof(bmpInfoHeader);
    image->matrix = NULL;
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
    if(image->matrix) oilColorMatrixFree(image->matrix);
    free(image->infoHeader);
    free(image->fileHeader);
    free(image);
}

bmpImage* createBMPImage(uint32_t width, uint32_t height, uint16_t bitDepth)
{
    bmpImage* image = allocImage();
    image->fileHeader->signature = bmp_signature_bm;
    image->fileHeader->offsetBytes = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader);
    image->fileHeader->fileSize = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) +
            (width + height) * 3 * bitDepth / 8;

    image->infoHeader->height = height;
    image->infoHeader->width = width;
    image->infoHeader->bitDepth = bitDepth;
    image->infoHeader->compression = bmp_compression_rgb;
    image->infoHeader->colorsImportant = 0;
    image->infoHeader->planes = 0;
    image->infoHeader->imageSize = 0;
    image->infoHeader->XpelsPerMeter = 100;
    image->infoHeader->YpelsPerMeter = 100;

    image->matrix = oilColorMatrixAlloc(1, width, height);

    return image;
}

uint8_t oilBMPSave(bmpImage* image, char* fileName)
{
    FILE* f = fopen(fileName, "w");

    if(!f) {
        oilPushErrorf("Unable to open file %s", fileName);
        return 0;
    }

    if(fwrite(image->fileHeader, sizeof(bmpFileHeader), 1, f) != 1)
    {
        oilPushError("Unable to write fileHeader");
        fclose(f);
        return 0;
    }

    if(fwrite(image->infoHeader, sizeof(bmpInfoHeader), 1, f) != 1)
    {
        oilPushError("Unable to write infoHeader");
        fclose(f);
        return 0;
    }

    for(uint32_t y = 0; y < image->infoHeader->width; y++)
        for(uint32_t x = 0; x < image->infoHeader->width; x++)
        {
            switch(image->infoHeader->bitDepth)
            {
                case 16:
                    if(fwrite(&image->matrix->matrix[y][x]->r, sizeof(uint16_t), 1, f) != 1) {
                        oilPushError("Unable to write color component");
                        fclose(f);
                        return 0;
                    }

                    if(fwrite(&image->matrix->matrix[y][x]->g, sizeof(uint16_t), 1, f) != 1) {
                        oilPushError("Unable to write color component");
                        fclose(f);
                        return 0;
                    }

                    if(fwrite(&image->matrix->matrix[y][x]->b, sizeof(uint16_t), 1, f) != 1) {
                        oilPushError("Unable to write color component");
                        fclose(f);
                        return 0;
                    }
                    break;

                case 8:
                default:
                {
                    uint8_t r = (uint8_t) image->matrix->matrix[y][x]->r;
                    uint8_t g = (uint8_t) image->matrix->matrix[y][x]->r;
                    uint8_t b = (uint8_t) image->matrix->matrix[y][x]->r;

                    if (fwrite(&r, sizeof(uint16_t), 1, f) != 1)
                    {
                        oilPushError("Unable to write color component");
                        fclose(f);
                        return 0;
                    }

                    if (fwrite(&g, sizeof(uint16_t), 1, f) != 1)
                    {
                        oilPushError("Unable to write color component");
                        fclose(f);
                        return 0;
                    }

                    if (fwrite(&b, sizeof(uint16_t), 1, f) != 1)
                    {
                        oilPushError("Unable to write color component");
                        fclose(f);
                        return 0;
                    }

                    break;
                }
            }
        }

    if(fclose(f) == -1)
    {
        oilPushError("Unable to close file");
        return 0;
    }

    return 1;
}