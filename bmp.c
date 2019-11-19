//
// Created by maxim on 3/18/19.
//

#include "bmp.h"

bmpImage* allocImage(uint16_t signature, bmpHeaderType type)
{
   bmpImage* image = malloc(sizeof(bmpImage));
   image->fileHeader = malloc(sizeof(bmpFileHeader));
   image->fileHeader->bfType = signature;
   image->fileHeader->bfReserved1 = 0;
   image->fileHeader->bfReserved2 = 0;

   image->imageData = malloc(sizeof(bmpImageData));
   image->imageData->headerType = type;

   switch (type)
   {
      case BITMAPCOREHEADER:
      default:
      {
         image->imageData->header = malloc(sizeof(bmpCoreHeader));
         bmpCoreHeader* header = image->imageData->header;
         header->bcSize = (uint32_t) bmpCoreHeaderSize;
         image->fileHeader->bfOffBits = (uint32_t) (bmpFileHeaderSize + bmpCoreHeaderSize);
         break;
      }

      case BITMAPINFOHEADER:
      {
         image->imageData->header = malloc(sizeof(bmpInfoHeader));
         bmpInfoHeader* header = image->imageData->header;
         header->biSize = (uint32_t) bmpInfoHeaderSize;
         image->fileHeader->bfOffBits = (uint32_t) (bmpFileHeaderSize + bmpInfoHeaderSize);
         break;
      }

      case BITMAPV4HEADER:
      {
         image->imageData->header = malloc(sizeof(bmpV4Header));
         bmpV4Header* header = image->imageData->header;
         header->bV4Size = (uint32_t) bmpV4HeaderSize;
         image->fileHeader->bfOffBits = (uint32_t) (bmpFileHeaderSize + bmpV4HeaderSize);
         break;
      }

      case BITMAPV5HEADER:
      {
         image->imageData->header = malloc(sizeof(bmpV5Header));
         bmpV5Header* header = image->imageData->header;
         header->bV5Size = (uint32_t) bmpV5HeaderSize;
         image->fileHeader->bfOffBits = (uint32_t) (bmpFileHeaderSize + bmpV5HeaderSize);
         break;
      }
   }

   image->colorMatrix = NULL;
   return image;
}

bmpImage* oilBMPLoad(char* fileName)
{
   FILE* f = fopen(fileName, "rb");

   if (!f)
   {
      oilPushErrorf("[OILERROR]: Unable to open file \"%s\"\n", fileName);
      return 0;
   }


}

void oilBMPCalcCT(bmpImage* image, uint32_t colorLen, oilColor* colorTable)
{
   //image->
}

void oilBMPSetCT(bmpImage* image, oilColor* colorTable)
{
   image->imageData->colorTable = colorTable;
}

void oilBMPFreeImage(bmpImage* image)
{
   if (image->colorMatrix) oilColorMatrixFree(image->colorMatrix);
   free(image->imageData->header);
   free(image->imageData);
   free(image->fileHeader);
   free(image);
}

bmpImage* oilBMPCreateImageExt(uint32_t width, uint32_t height, uint16_t bitDepth, bmpHeaderType headerType)
{
   uint8_t correctBitDepth = 0;
   for (size_t i = 0; i < sizeof(allowedBMPBitDepth) / sizeof(allowedBMPBitDepth[0]); i++)
      if (allowedBMPBitDepth[i] == bitDepth)
      {
         correctBitDepth = 1;
         break;
      }

   if (!correctBitDepth)
   {
      oilPushErrorf("[OILERROR]: %i is wrong bitDepth value", bitDepth);
      return NULL;
   }

   if (headerType == BITMAPCOREHEADER && (bitDepth != 1 && bitDepth != 4 && bitDepth != 8 && bitDepth != 24))
   {
      oilPushErrorf("[OILERROR]: %i is wrong bitDepth value for BITMAPCOREHEADER. Allowed values: 1, 4, 8, 24",
                    bitDepth);
      return NULL;
   }


   bmpImage* image = allocImage(bmp_signature_bm, headerType);
   image->colorMatrix = oilColorMatrixAlloc(1, width, height);

   uint32_t toPad = (4 - (width * 3) % 4) % 4;
   uint32_t imageSize = (width * height) * 3 * bitDepth / 24 + (width) * toPad;
   image->fileHeader->bfSize = image->fileHeader->bfOffBits + imageSize;

   switch (headerType)
   {
      case BITMAPCOREHEADER:
      default:
      {
         bmpCoreHeader* header = image->imageData->header;
         header->bcWidth = (uint16_t) width;
         header->bcHeight = (uint16_t) height;
         header->bcBitCount = bitDepth;
         header->bcPlanes = 0;
         break;
      }

      case BITMAPINFOHEADER:
      {
         bmpInfoHeader* header = image->imageData->header;
         header->biSizeImage = imageSize;

         header->biWidth = width;
         header->biHeight = height;

         header->biBitCount = bitDepth;
         header->biCompression = bmp_compression_rgb;
         header->biPlanes = 1;
         header->biXPelsPerMeter = 2835;
         header->biYPelsPerMeter = 2835;

         header->biClrImportant = 0;
         header->biClrUsed = 0;
         break;
      }

      case BITMAPV4HEADER:
      {
         bmpV4Header* header = image->imageData->header;
         break;
      }

      case BITMAPV5HEADER:
      {
         bmpV5Header* header = image->imageData->header;
         break;
      }
   }

   if (headerType == BITMAPCOREHEADER)
   {
      image->width = (uint16_t) width;
      image->height = (uint16_t) height;
   }
   else
   {
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

   if (!f)
   {
      oilPushErrorf("Unable to open file %s", fileName);
      return 0;
   }

   writeData(image->fileHeader->bfType, "fileHeader: signature");
   writeData(image->fileHeader->bfSize, "fileHeader: fileSize");
   writeData(image->fileHeader->bfReserved1, "fileHeader: reserved1");
   writeData(image->fileHeader->bfReserved2, "fileHeader: reserved2");
   writeData(image->fileHeader->bfOffBits, "fileHeader: offsetBytes");
   uint16_t bitDepth = 0;

   switch (image->imageData->headerType)
   {
      case BITMAPCOREHEADER:
      default:
      {
         bmpCoreHeader* header = image->imageData->header;
         writeData(header->bcSize, "infoHeader : headerSize");
         writeData(header->bcWidth, "infoHeader : width");
         writeData(header->bcHeight, "infoHeader : height");
         writeData(header->bcPlanes, "infoHeader : planes");
         writeData(header->bcBitCount, "infoHeader : bitDepth");
         bitDepth = header->bcBitCount;
         break;
      }
      case BITMAPINFOHEADER:
      {
         bmpInfoHeader* header = image->imageData->header;
         writeData(header->biSize, "infoHeader : headerSize");
         writeData(header->biWidth, "infoHeader : width");
         writeData(header->biHeight, "infoHeader : height");
         writeData(header->biPlanes, "infoHeader : planes");
         writeData(header->biBitCount, "infoHeader : bitDepth");
         writeData(header->biCompression, "infoHeader : compression");
         writeData(header->biSizeImage, "infoHeader : imageSize");
         writeData(header->biXPelsPerMeter, "infoHeader : XpelsPerMeter");
         writeData(header->biYPelsPerMeter, "infoHeader : YpelsPerMeter");
         writeData(header->biClrUsed, "infoHeader : colorsUsed");
         writeData(header->biClrImportant, "infoHeader : colorsImportant");
         bitDepth = header->biBitCount;
         break;
      }
   }

   int toPad = (4 - (image->width * 3) % 4) % 4;

   uint8_t paddingBytes1 = 0;
   uint16_t paddingBytes2 = 0;

   for (int32_t y = image->height - 1; y >= 0; y--)
   {
      for (uint32_t x = 0; x < image->width; x++)
      {
         switch (bitDepth)
         {
            case 32:
            {
               uint8_t r = (uint8_t) image->colorMatrix->matrix[y][x]->r;
               uint8_t g = (uint8_t) image->colorMatrix->matrix[y][x]->g;
               uint8_t b = (uint8_t) image->colorMatrix->matrix[y][x]->b;
               uint8_t a = (uint8_t) image->colorMatrix->matrix[y][x]->a;

               writeData(b, "colorComponent: b");
               writeData(g, "colorComponent: g");
               writeData(r, "colorComponent: r");
               writeData(a, "colorComponent: a");
               break;
            }
            case 24:
            default:
            {
               uint8_t r = (uint8_t) image->colorMatrix->matrix[y][x]->r;
               uint8_t g = (uint8_t) image->colorMatrix->matrix[y][x]->g;
               uint8_t b = (uint8_t) image->colorMatrix->matrix[y][x]->b;

               writeData(b, "colorComponent: b");
               writeData(g, "colorComponent: g");
               writeData(r, "colorComponent: r");

               break;
            }
            case 16:
            case 8:
            case 4:
            case 2:
            case 1:
               //colortable
               break;
         }

      }

      switch (toPad)
      {
         case 1:
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

   if (fclose(f) == -1)
   {
      oilPushError("Unable to close file");
      return 0;
   }

   return 1;
}