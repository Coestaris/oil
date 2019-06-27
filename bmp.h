//
// Created by maxim on 3/18/19.
//

#ifndef OIL_BMP_H
#define OIL_BMP_H

#include "common.h"
#include "oilerror.h"
#include "bmpSizes.h"

static uint16_t bmp_signature_bm  = 0x4D42; //Windows 3.1x, 95, NT, ... etc.
static uint16_t bmp_signature_ba  = 0x4142; //OS/2 struct bitmap array
static uint16_t bmp_signature_ci  = 0x4F43; //OS/2 struct color icon
static uint16_t bmp_signature_cp  = 0x5042; //OS/2 const color pointer
static uint16_t bmp_signature_ic  = 0x4349; //OS/2 struct icon
static uint16_t bmp_signature_ptb = 0x5450; //OS/2 pointer

static uint32_t bmp_compression_rgb             = 0;
static uint32_t bmp_compression_rle8            = 1;
static uint32_t bmp_compression_rle4            = 2;
static uint32_t bmp_compression_bitFields       = 3;
static uint32_t bmp_compression_jpeg            = 4;
static uint32_t bmp_compression_png             = 5;
static uint32_t bmp_compression_alphaBitFields  = 6;
static uint32_t bmp_compression_cmyk            = 11;
static uint32_t bmp_compression_cmyk_rle8       = 12;
static uint32_t bmp_compression_cmyk_rle4       = 13;

typedef enum {
    BITMAPCOREHEADER,
    BITMAPINFOHEADER,

    BITMAPV4HEADER,
    BITMAPV5HEADER

} bmpHeaderType;

typedef struct {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;

} bmpFileHeader;

typedef struct {
    DWORD   bcSize;
    WORD    bcWidth;
    WORD    bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;

} bmpCoreHeader;

typedef struct {
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;

} bmpInfoHeader;

typedef struct {
    DWORD        bV4Size;
    LONG         bV4Width;
    LONG         bV4Height;
    WORD         bV4Planes;
    WORD         bV4BitCount;
    DWORD        bV4V4Compression;
    DWORD        bV4SizeImage;
    LONG         bV4XPelsPerMeter;
    LONG         bV4YPelsPerMeter;
    DWORD        bV4ClrUsed;
    DWORD        bV4ClrImportant;
    DWORD        bV4RedMask;
    DWORD        bV4GreenMask;
    DWORD        bV4BlueMask;
    DWORD        bV4AlphaMask;
    DWORD        bV4CSType;
    CIEXYZTRIPLE bV4Endpoints;
    DWORD        bV4GammaRed;
    DWORD        bV4GammaGreen;
    DWORD        bV4GammaBlue;

} bmpV4Header;

typedef struct {
    DWORD        bV5Size;
    LONG         bV5Width;
    LONG         bV5Height;
    WORD         bV5Planes;
    WORD         bV5BitCount;
    DWORD        bV5Compression;
    DWORD        bV5SizeImage;
    LONG         bV5XPelsPerMeter;
    LONG         bV5YPelsPerMeter;
    DWORD        bV5ClrUsed;
    DWORD        bV5ClrImportant;
    DWORD        bV5RedMask;
    DWORD        bV5GreenMask;
    DWORD        bV5BlueMask;
    DWORD        bV5AlphaMask;
    DWORD        bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    DWORD        bV5GammaRed;
    DWORD        bV5GammaGreen;
    DWORD        bV5GammaBlue;
    DWORD        bV5Intent;
    DWORD        bV5ProfileData;
    DWORD        bV5ProfileSize;
    DWORD        bV5Reserved;

} bmpV5Header;

typedef struct {
    bmpHeaderType headerType;
    void* header;

} bmpImageData;

typedef struct {
    bmpFileHeader* fileHeader;
    bmpImageData* imageData;

    uint32_t width;
    uint32_t height;

    colorMatrix* colorMatrix;

} bmpImage;

bmpImage* oilBMPCreateImage(uint16_t width, uint16_t height, uint16_t bitDepth);
bmpImage* oilBMPCreateImageExt(uint32_t width, uint32_t height, uint16_t bitDepth, bmpHeaderType headerType);

uint8_t oilBMPSave(bmpImage* image, char* fileName);
//bmpImage* oilBMPLoad(char* filename);
void oilBMPFreeImage(bmpImage* image);

#endif //OIL_BMP_H
