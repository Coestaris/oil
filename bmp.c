//
// Created by maxim on 3/18/19.
//

#include "bmp.h"

bmpImage* allocImage(uint16_t signature, bmpHeaderType type)
{
    bmpImage* image = malloc(sizeof(bmpImage));
    image->fileHeader = malloc(sizeof(bmpFileHeader));
    image->fileHeader->signature = signature;
    image->fileHeader->reserved1 = 0;
    image->fileHeader->reserved2 = 0;

    image->imageData = malloc(sizeof(bmpImageData));
    image->imageData->headerType = type;

    switch(type)
    {
        case BITMAPCOREHEADER:
        default:
        {
            image->imageData->header = malloc(sizeof(bmpCoreHeader));
            bmpCoreHeader* header = image->imageData->header;
            header->headerSize = (uint32_t)bmpCoreHeaderSize;
            image->fileHeader->offsetBytes = (uint32_t)(bmpFileHeaderSize + bmpCoreHeaderSize);
            break;
        }

        case BITMAPINFOHEADER:
        {
            image->imageData->header = malloc(sizeof(bmpInfoHeader));
            bmpInfoHeader* header = image->imageData->header;
            header->headerSize = (uint32_t)bmpInfoHeaderSize;
            image->fileHeader->offsetBytes = (uint32_t)(bmpFileHeaderSize + bmpInfoHeaderSize);
            break;
        }

        case BITMAPV4HEADER:
        {
            image->imageData->header = malloc(sizeof(bmpV4Header));
            bmpV4Header* header = image->imageData->header;
            header->headerSize = (uint32_t)bmpV4HeaderSize;
            image->fileHeader->offsetBytes = (uint32_t)(bmpFileHeaderSize + bmpV4HeaderSize);
            break;
        }
    }

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
    free(image->imageData->header);
    free(image->imageData);
    free(image->fileHeader);
    free(image);
}

bmpImage* oilBMPCreateImageExt(uint32_t width, uint32_t height, uint16_t bitDepth, bmpHeaderType headerType)
{
    bmpImage* image = allocImage(bmp_signature_bm, headerType);
    image->colorMatrix = oilColorMatrixAlloc(1, width, height);

    uint32_t toPad = (4 - (width * 3) % 4) % 4;
    uint32_t imageSize = (width * height) * 3 * bitDepth / 24 + (width)* toPad;
    image->fileHeader->fileSize = image->fileHeader->offsetBytes  + imageSize;

    switch(headerType)
    {
        case BITMAPCOREHEADER:
        default:
        {
            bmpCoreHeader* header = image->imageData->header;
            header->width = (uint16_t)width;
            header->height = (uint16_t)height;
            header->bitDepth = bitDepth;
            header->planes = 0;
            break;
        }

        case BITMAPINFOHEADER:
        {
            bmpInfoHeader* header = image->imageData->header;
            header->imageSize = imageSize;

            header->width = width;
            header->height = height;

            header->bitDepth = bitDepth;
            header->compression = bmp_compression_rgb;
            header->colorsImportant = 0;
            header->planes = 1;
            header->XpelsPerMeter = 2835;
            header->YpelsPerMeter = 2835;

            header->colorsImportant = 0;
            header->colorsUsed = 0;
            break;
        }

        case BITMAPV4HEADER:
        {
            bmpV4Header* header = image->imageData->header;
            break;
        }
    }

    if(headerType == BITMAPCOREHEADER) {
        image->width = (uint16_t)width;
        image->height = (uint16_t)height;
    } else {
        image->width = width;
        image->height = height;
    }

    return image;
}

bmpImage* oilBMPCreateImage(uint16_t width, uint16_t height, uint16_t bitDepth)
{
    return oilBMPCreateImageExt(width, height, bitDepth, BITMAPCOREHEADER);
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
    uint16_t bitDepth = 0;

    switch(image->imageData->headerType)
    {
        case BITMAPCOREHEADER:
        default:
        {
            bmpCoreHeader* header = image->imageData->header;
            writeData(header->headerSize,       "infoHeader : headerSize");
            writeData(header->width,             "infoHeader : width");
            writeData(header->height,            "infoHeader : height");
            writeData(header->planes,           "infoHeader : planes");
            writeData(header->bitDepth,         "infoHeader : bitDepth");
            bitDepth = header->bitDepth;
            break;
        }
        case BITMAPINFOHEADER:
        {
            bmpInfoHeader* header = image->imageData->header;
            writeData(header->headerSize,      "infoHeader : headerSize");
            writeData(image->width,            "infoHeader : width");
            writeData(image->height,           "infoHeader : height");
            writeData(header->planes,          "infoHeader : planes");
            writeData(header->bitDepth,        "infoHeader : bitDepth");
            writeData(header->compression,     "infoHeader : compression");
            writeData(header->imageSize,       "infoHeader : imageSize");
            writeData(header->XpelsPerMeter,   "infoHeader : XpelsPerMeter");
            writeData(header->YpelsPerMeter,   "infoHeader : YpelsPerMeter");
            writeData(header->colorsUsed,      "infoHeader : colorsUsed");
            writeData(header->colorsImportant, "infoHeader : colorsImportant");
            bitDepth = header->bitDepth;
            break;
        }
    }

    int toPad = (4 - (image->width * 3) % 4) % 4;

    uint8_t  paddingBytes1;
    uint16_t paddingBytes2;

    for(int32_t y = image->height - 1; y >= 0; y--)
    {
        for (uint32_t x = 0; x < image->width; x++)
        {
            switch (bitDepth)
            {
                case 16:
                    writeData(image->colorMatrix->matrix[y][x]->r, "colorComponent: r");
                    writeData(image->colorMatrix->matrix[y][x]->g, "colorComponent: g");
                    writeData(image->colorMatrix->matrix[y][x]->b, "colorComponent: b");
                    break;

                case 8:
                default:
                {
                    uint8_t r = (uint8_t)image->colorMatrix->matrix[y][x]->r;
                    uint8_t g = (uint8_t)image->colorMatrix->matrix[y][x]->g;
                    uint8_t b = (uint8_t)image->colorMatrix->matrix[y][x]->b;

                    writeData(b, "colorComponent: b");
                    writeData(g, "colorComponent: g");
                    writeData(r, "colorComponent: r");

                    break;
                }
            }

        }

        switch(toPad)
        {
            case 1:
            default:
                writeData(paddingBytes1, "paddingBytes");
                break;
            case 2:
                writeData(paddingBytes2, "paddingBytes");
                break;
            case 3:
                writeData(paddingBytes1, "paddingBytes");
                writeData(paddingBytes2, "paddingBytes");
                break;
        }
    }

    if(fclose(f) == -1)
    {
        oilPushError("Unable to close file");
        return 0;
    }

    return 1;
}