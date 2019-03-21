//
// Created by maxim on 3/18/19.
//

#ifndef OIL_BMP_H
#define OIL_BMP_H

#include "common.h"

static uint16_t bmp_signature_bm  = 0x424D; //Windows 3.1x, 95, NT, ... etc.
static uint16_t bmp_signature_ba  = 0x4241; //OS/2 struct bitmap array
static uint16_t bmp_signature_ci  = 0x434F; //OS/2 struct color icon
static uint16_t bmp_signature_cp  = 0x4250; //OS/2 const color pointer
static uint16_t bmp_signature_ic  = 0x4943; //OS/2 struct icon
static uint16_t bmp_signature_ptb = 0x5054; //OS/2 pointer

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

typedef struct {
    uint16_t signature;
    uint32_t fileSize;

    uint16_t reserved1;
    uint16_t reserved2;

    uint32_t  offsetBytes;

} bmpFileHeader;

typedef struct {
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;

    uint16_t planes;

    uint16_t bitDepth;

    uint32_t compression;
    uint32_t imageSize;

    uint32_t XpelsPerMeter;
    uint32_t YpelsPerMeter;

    uint32_t colorsUsed;
    uint32_t colorsImportant;

} bmpInfoHeader;


typedef struct {
    bmpFileHeader* fileHeader;
    bmpInfoHeader* infoHeader;

    colorMatrix* matrix;

} bmpImage;

bmpImage* createBMPImage(uint32_t width, uint32_t height, uint16_t bitDepth);

uint8_t oilBMPSave(bmpImage* image, char* fileName);
//bmpImage* oilBMPLoad(char* filename);
void oilBMPFreeImage(bmpImage* image);

#endif //OIL_BMP_H
