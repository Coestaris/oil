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
    0x0A             // LF
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

} pngchunk;

typedef struct
{
    uint16_t greyScale;

    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint8_t a;

} png_color;

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

    png_color bkgColor;

    size_t paletteLen;
    png_color* palette;

} png_color_management;

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;

    png_color_management color_management;
    uint8_t colorFlag;

    uint8_t compression;
    uint8_t filtration;
    uint8_t interlace;

    char* text;

    size_t chunksCount;
    pngchunk** chunks;

    uint8_t* data;
    size_t dataLen;

} pngimage;

typedef struct
{
    uint8_t compMethod;
    uint8_t compInfo;

    uint8_t fLevel;
    uint8_t fDict;
    uint8_t fCheck;

    uint32_t dict;

} zlib_header;

pngimage* oilCreateImg(void);
char* oilGetChunkName(pngchunk* chunk);
int oilProceedChunk(pngimage* image, pngchunk* chunk);
int oilGetChunks(char* fileName);

int oilProceedIDAT(pngimage* image, uint8_t * data, size_t length);

#endif //OIL_PNG_H
