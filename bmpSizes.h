//
// Created by maxim on 6/27/19.
//

#ifndef OIL_BMPSIZES_H
#define OIL_BMPSIZES_H

#include <stdint.h>
#include <stddef.h>

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint8_t CIEXYZTRIPLE[0x24];

static const size_t bmpFileHeaderSize
        = sizeof(WORD)         //bfType
          + sizeof(DWORD)         //bfSize
          + sizeof(WORD)         //bfReserved1
          + sizeof(WORD)         //bfReserved2
          + sizeof(DWORD);        //bfOffBits

static const size_t bmpCoreHeaderSize
        = sizeof(DWORD)         //bcSize
          + sizeof(WORD)         //bcWidth
          + sizeof(WORD)         //bcHeight
          + sizeof(WORD)         //bcPlanes
          + sizeof(WORD);        //bcBitCount


static const size_t bmpInfoHeaderSize
        = sizeof(DWORD)         //biSize
          + sizeof(LONG)         //biWidth
          + sizeof(LONG)         //biHeight
          + sizeof(WORD)         //biPlanes
          + sizeof(WORD)         //biBitCount
          + sizeof(DWORD)         //biCompression
          + sizeof(DWORD)         //biSizeImage
          + sizeof(LONG)         //biXPelsPerMeter
          + sizeof(LONG)         //biYPelsPerMeter
          + sizeof(DWORD)         //biClrUsed
          + sizeof(DWORD);        //biClrImportant

static const size_t bmpV4HeaderSize
        = sizeof(DWORD)         //bV4Size
          + sizeof(LONG)         //bV4Width
          + sizeof(LONG)         //bV4Height
          + sizeof(WORD)         //bV4Planes
          + sizeof(WORD)         //bV4BitCount
          + sizeof(DWORD)         //bV4V4Compression
          + sizeof(DWORD)         //bV4SizeImage
          + sizeof(LONG)         //bV4XPelsPerMeter
          + sizeof(LONG)         //bV4YPelsPerMeter
          + sizeof(DWORD)         //bV4ClrUsed
          + sizeof(DWORD)         //bV4ClrImportant
          + sizeof(DWORD)         //bV4RedMask
          + sizeof(DWORD)         //bV4GreenMask
          + sizeof(DWORD)         //bV4BlueMask
          + sizeof(DWORD)         //bV4AlphaMask
          + sizeof(DWORD)         //bV4CSType
          + sizeof(CIEXYZTRIPLE)  //bV4Endpoints
          + sizeof(DWORD)         //bV4GammaRed
          + sizeof(DWORD)         //bV4GammaGreen
          + sizeof(DWORD);        //bV4GammaBlue

static const size_t bmpV5HeaderSize
        = sizeof(DWORD)         //bV5Size
          + sizeof(LONG)         //bV5Width
          + sizeof(LONG)         //bV5Height
          + sizeof(WORD)         //bV5Planes
          + sizeof(WORD)         //bV5BitCount
          + sizeof(DWORD)         //bV5Compression
          + sizeof(DWORD)         //bV5SizeImage
          + sizeof(LONG)         //bV5XPelsPerMeter
          + sizeof(LONG)         //bV5YPelsPerMeter
          + sizeof(DWORD)         //bV5ClrUsed
          + sizeof(DWORD)         //bV5ClrImportant
          + sizeof(DWORD)         //bV5RedMask
          + sizeof(DWORD)         //bV5GreenMask
          + sizeof(DWORD)         //bV5BlueMask
          + sizeof(DWORD)         //bV5AlphaMask
          + sizeof(DWORD)         //bV5CSType
          + sizeof(CIEXYZTRIPLE)  //bV5Endpoints
          + sizeof(DWORD)         //bV5GammaRed
          + sizeof(DWORD)         //bV5GammaGreen
          + sizeof(DWORD)         //bV5GammaBlue
          + sizeof(DWORD)         //bV5Intent
          + sizeof(DWORD)         //bV5ProfileData
          + sizeof(DWORD)         //bV5ProfileSize
          + sizeof(DWORD);        //bV5Reserved


#endif //OIL_BMPSIZES_H
