//
// Created by maxim on 3/9/19.
//

#ifndef OIL_PNG_H
#define OIL_PNG_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <math.h>

#include "common.h"
#include "oilerror.h"
#include "crc32.h"

#define OIL_DONT_IGNORE_CHUNKS

#define OILDEBUG_PRINT_CHUNK_NAMES
#define OILDEBUG_PRINT_COMPRESSED_DATA
#define OILDEBUG_PRINT_DECOMPRESSED_DATA
#define OILDEBUG_PRINT_SCANLINES

static uint8_t png_signature[8] = {
    0x89,             // Non ASCII symbol
    0x50, 0x4E, 0x47, // "PNG"
    0x0D, 0x0A,       // CRLF
    0x1A,             // end-of-file symbol
    0x0A              // LF
};

static uint32_t png_chunk_IEND = 0x444E4549u; //[4] = { 0x49, 0x45, 0x4E, 0x44 };
static uint32_t png_chunk_IHDR = 0X52444849u; //[4] = { 0x49, 0x48, 0x44, 0x52 };
static uint32_t png_chunk_gAMA = 0x414D4167u; //[4] = { 0x67, 0x41, 0x4D, 0x41 };
static uint32_t png_chunk_cHRM = 0x4D524863u; //[4] = { 0x63, 0x48, 0x52, 0x4D };
static uint32_t png_chunk_bKGD = 0x44474B62u; //[4] = { 0x62, 0x4B, 0x47, 0x44 };
static uint32_t png_chunk_IDAT = 0x54414449u; //[4] = { 0x49, 0x44, 0x41, 0x54 };
static uint32_t png_chunk_tEXt = 0x74584574u; //[4] = { 0x74, 0x45, 0x58, 0x74 };
static uint32_t png_chunk_PLTE = 0x45544C50u; //[4] = { 0x50, 0x4C, 0x54, 0x45 };
static uint32_t png_chunk_pHYs = 0x73594870u; //[4] = { 0x70, 0x48, 0x59, 0x73 };
static uint32_t png_chunk_tIME = 0x454D4974u; //[4] = { 0x74, 0x49, 0x4D, 0x45 };
static uint32_t png_chunk_iTXt = 0x74585469u; //[4] = { 0x69, 0x54, 0x58, 0x74 };

#define png_filterType_none    0
#define png_filterType_sub     1
#define png_filterType_up      2
#define png_filterType_average 3
#define png_filterType_paeth   4

#define png_get_next_byte data[(*byteCounter)++]
#define png_colorflag_palette(byte) (uint8_t)((byte & 0b00000001) != 0)
#define png_colorflag_color(byte)   (uint8_t)((byte & 0b00000010) != 0)
#define png_colorflag_alpha(byte)   (uint8_t)((byte & 0b00000100) != 0)

typedef struct
{
    uint32_t length;
    uint32_t type;
    uint8_t* data;
    uint32_t crc;

} pngChunk;

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} pngTime;

typedef struct
{
    char* key;
    char* value;

} pngText;

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

    oilColor bkgColor;

    size_t paletteLen;
    oilColor* palette;

    uint32_t ppuX;
    uint32_t ppuY;

} pngPixelData;

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint8_t bitDepth;

    pngPixelData* pixelsInfo;
    uint8_t colorFlag;

    uint8_t compression;
    uint8_t filtration;
    uint8_t interlace;

    int txtItemsCount;
    pngText* txtItems;
    pngTime* time;

    colorMatrix* colorMatrix;

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

pngImage* oilPNGLoad(char *fileName, int simplified);
void oilPNGFreeImage(pngImage *image);

#endif //OIL_PNG_H

