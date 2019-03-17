//
// Created by maxim on 3/9/19.
//

#ifndef OIL_PNG_H
#define OIL_PNG_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>

#include "oilerror.h"
#include "crc32.h"

static uint8_t png_signature[8] = {
    0x89,             //Non ASCII symbol
    0x50, 0x4E, 0x47, //"PNG"
    0x0D, 0x0A,       //CRLF
    0x1A,             // end-of-file symbol
    0x0A              // LF
};

static uint8_t png_chunk_end[4]   = { 0x49, 0x45, 0x4E, 0x44 };
static uint8_t png_chunk_start[4] = { 0x49, 0x48, 0x44, 0x52 };
static uint8_t png_chunk_gAMA[4]  = { 0x67, 0x41, 0x4D, 0x41 };
static uint8_t png_chunk_cHRM[4]  = { 0x63, 0x48, 0x52, 0x4D };
static uint8_t png_chunk_bKGD[4]  = { 0x62, 0x4B, 0x47, 0x44 };
static uint8_t png_chunk_IDAT[4]  = { 0x49, 0x44, 0x41, 0x54 };
static uint8_t png_chunk_tEXt[4]  = { 0x74, 0x45, 0x58, 0x74 };
static uint8_t png_chunk_PLTE[4]  = { 0x50, 0x4C, 0x54, 0x45 };

typedef struct
{
    uint32_t length;
    uint32_t type;
    uint8_t* data;
    uint32_t crc;

} pngChunk;

typedef struct
{
    char* key;
    char* value;

} pngText;

typedef struct
{
    uint16_t greyScale;

    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint8_t a;

} pngColor;

typedef struct
{
    uint8_t usePalette;
    uint8_t useColor;
    uint8_t hasAlpha;

    uint8_t gammaSet;
    uint32_t gamma;

    uint8_t cieSet;
    uint32_t whitePointX;
    uint32_t whitePointY;
    uint32_t redX;
    uint32_t redY;
    uint32_t greenX;
    uint32_t greenY;
    uint32_t blueX;
    uint32_t blueY;

    uint8_t bkgColorSet;

    pngColor bkgColor;

    size_t paletteLen;
    pngColor* palette;

} pngPixelData;

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint8_t bitDepth;

    pngPixelData* pixelData;
    uint8_t colorFlag;

    uint8_t compression;
    uint8_t filtration;
    uint8_t interlace;

    int txtItemsCount;
    pngText* txtItems;

    pngColor** colors;

} pngImage;

/*typedef struct
{
    uint8_t compMethod;
    uint8_t compInfo;

    uint8_t fLevel;
    uint8_t fDict;
    uint8_t fCheck;

    uint32_t dict;

} zlib_header;*/

pngImage* oilCreateImg(void);
char* oilGetChunkName(pngChunk* chunk);
int oilProceedChunk(pngImage* image, pngChunk* chunk);
int oilLoadImage(char *fileName, pngImage *image);
pngImage* oilLoad(char *fileName);

void oilFreeImage(pngImage* image);
void oilGetImageData(pngImage* image, int format);

void oilColorMatrixFree(pngImage* image);
void oilColorMatrixCreate();

void printColor(pngColor color, int flag);
void getImageColors(pngImage *image, size_t *byteCounter, uint8_t *data);
int oilProceedIDAT(pngImage* image, uint8_t * data, size_t length);

#endif //OIL_PNG_H
