//
// Created by maxim on 3/9/19.
//

#include "png.h"

uint8_t buff4[4];

uint32_t buffToU32(const uint8_t* buff)
{
   return (uint32_t) buff[3] |
          (uint32_t) buff[2] << 8 |
          (uint32_t) buff[1] << 16 |
          (uint32_t) buff[0] << 24;
}

uint16_t buffToU16(const uint8_t* buff)
{
   return (uint16_t) buff[1] << 0 |
          (uint16_t) buff[0] << 8;
}

pngImage* oilCreateImg(void)
{
   pngImage* img = OIL_MALLOC(sizeof(pngImage));
   img->pixelsInfo = OIL_MALLOC(sizeof(pngPixelData));
   img->pixelsInfo->cie = NULL;
   img->pixelsInfo->bkgColor = NULL;
   img->pixelsInfo->ppuY = 0;
   img->pixelsInfo->ppuX = 0;
   img->pixelsInfo->gammaSet = 0;
   img->pixelsInfo->palette = NULL;

   img->imageData = OIL_MALLOC(sizeof(pngImageData));
   img->imageData->txtItems = NULL;
   img->imageData->rawData = NULL;
   img->imageData->time = NULL;

   img->colorMatrix = NULL;
   return img;
}

char* oilGetChunkName(pngChunk* chunk)
{
   char* str = OIL_MALLOC(5);
   for (size_t i = 0; i < 4; i++)
   {
      str[i] = (char) ((chunk->type >> i * 8) & 0xFF);
   }
   str[4] = '\0';
   return str;
}


void getImageColors(pngImage* image, size_t* byteCounter, uint8_t* data, size_t scanlineIndex)
{
   for (size_t i = 0; i < image->width; i++)
   {
      oilColor* color = image->colorMatrix->matrix[scanlineIndex][i];

      if (image->pixelsInfo->usePalette)
      {
         color = image->pixelsInfo->palette[png_get_next_byte];
      }
      else
      {
         if (image->pixelsInfo->useColor)
         {
            if (image->pixelsInfo->bitDepth == 16)
            {
               color->r = png_get_next_byte << 8 | png_get_next_byte;
               color->g = png_get_next_byte << 8 | png_get_next_byte;
               color->b = png_get_next_byte << 8 | png_get_next_byte;
            }
            else
            {
               color->r = png_get_next_byte;
               color->g = png_get_next_byte;
               color->b = png_get_next_byte;
            }
         }
         else
         {
            if (image->pixelsInfo->bitDepth == 16)
            {
               color->r = color->g = color->b = png_get_next_byte << 8 | png_get_next_byte;
            }
            else
            {
               color->r = color->g = color->b = png_get_next_byte;
            }
         }
      }

      if (image->pixelsInfo->hasAlpha)
      {
         if (image->pixelsInfo->bitDepth == 16)
         {
            color->a = png_get_next_byte << 8 | png_get_next_byte;
         }
         else
         {
            color->a = png_get_next_byte;
         }
      }
      else
      {
         color->a = (image->pixelsInfo->bitDepth == 16) ? (uint16_t) 0xFFFF : (uint8_t) 0xFF;
      }

      image->colorMatrix->matrix[scanlineIndex][i] = color;
   }
}

void filterData(pngImage* image, uint8_t* output, uint8_t bytesPerColor, size_t i,
                uint8_t (* filterFunc)(int x, int a, int b, int c))
{
   for (size_t color = 0; color < image->width; color++)
   {
      for (size_t component = 0; component < bytesPerColor; component++)
      {
         int x = output[(i * bytesPerColor * image->width + i + 1) + color * bytesPerColor + component];
         int a = (color != 0) ? (output[(i * bytesPerColor * image->width + i + 1) + (color - 1) * bytesPerColor +
                                        component]) : (0);
         int b = (i != 0) ? (output[((i - 1) * bytesPerColor * image->width + i) + color * bytesPerColor + component])
                          : (0);
         int c = (i != 0 && color != 0) ? (output[((i - 1) * bytesPerColor * image->width + i) +
                                                  (color - 1) * bytesPerColor + component]) : (0);

         int z = (i * bytesPerColor * image->width + i + 1) + color * bytesPerColor + component;
         //printf("index try: %d, b: %d\n", z, ((i - 1) * bytesPerColor * image->width + i) + color * bytesPerColor + component);
         output[z] = filterFunc(x, a, b, c);
         //printf("x: %5i, a: %5i, b: %5i, c: %5i\n", x, a, b, c);
      }
   }
}

uint8_t filterSub(int x, int a, int b, int c)
{
   return (uint8_t) (x + a);
}

uint8_t filterUp(int x, int a, int b, int c)
{
   return (uint8_t) (x + b);
}

uint8_t filterAverage(int x, int a, int b, int c)
{
   return (uint8_t) (x + floor((a + b) / 2.0));
}

uint8_t filterPaeth(int x, int a, int b, int c)
{
   int pr = 0;
   int p = a + b - c;
   int pa = abs(p - a);
   int pb = abs(p - b);
   int pc = abs(p - c);

   if (pa <= pb && pa <= pc) pr = a;
   else if (pb <= pc) pr = b;
   else pr = c;

   return (uint8_t) (x + pr);
}

int oilProceedIDAT(pngImage* image, uint8_t* data, size_t length)
{
#ifdef OILDEBUG_PRINT_COMPRESSED_DATA
   printf("[OILDEBUG]: Compressed image data (%li bytes):\n", length);
   for(int i = 0; i < length; i++) {
       printf("%.2X ", data[i]);
       if((i + 1) % 30 == 0) putchar('\n');
   }
   putchar('\n');
#endif

   if (!image->imageData->rawData)
   {
      image->imageData->rawDataLength = length;
      image->imageData->rawData = OIL_MALLOC(length);
      memcpy(image->imageData->rawData, data, length);
   }
   else
   {
      image->imageData->rawData = realloc(image->imageData->rawData, image->imageData->rawDataLength + length);
      memcpy(image->imageData->rawData + image->imageData->rawDataLength, data, length);
      image->imageData->rawDataLength += length;
   }
}

uint8_t proceedImageData(pngImage* image)
{
   uint8_t bytesPerColor = 0;
   if (image->pixelsInfo->usePalette)
   {
      bytesPerColor = 1;
   }
   else if (image->pixelsInfo->useColor)
   {
      bytesPerColor = (uint8_t) (3 * image->pixelsInfo->bitDepth / 8);
   }
   else
   {
      bytesPerColor = (uint8_t) (image->pixelsInfo->bitDepth / 8);
   }

   if (image->pixelsInfo->hasAlpha)
   {
      bytesPerColor += image->pixelsInfo->bitDepth / 8;
   }

   size_t outputLen = bytesPerColor * image->width * image->height + image->height;
   uint8_t* output = OIL_MALLOC(sizeof(uint8_t) * outputLen);

   z_stream infstream;
   infstream.zalloc = Z_NULL;
   infstream.zfree = Z_NULL;
   infstream.opaque = Z_NULL;

   infstream.avail_in = (uInt) image->imageData->rawDataLength;
   infstream.next_in = image->imageData->rawData;
   infstream.avail_out = (uInt) outputLen - 1;
   infstream.next_out = output;

   inflateInit(&infstream);

   int result;
   if ((result = inflate(&infstream, Z_NO_FLUSH)) != Z_OK)
   {
      OIL_FREE(output);
      oilPushErrorf("[OILERROR]: Unable to decompress data. ZLIB error: %s\n", zError(result));
      inflateEnd(&infstream);
      return 0;
   }

   inflateEnd(&infstream);

#ifdef OILDEBUG_PRINT_DECOMPRESSED_DATA
   printf("[OILDEBUG]: Decompressed image data (%li bytes):\n", outputLen);
   for (int i = 0; i < outputLen; i++)
   {
       printf("%.2X ", output[i]);
       if ((i + 1) % 30 == 0) putchar('\n');
   }
   putchar('\n');
#endif

   image->colorMatrix = oilColorMatrixAlloc(1, image->width, image->height);

   size_t byteCounter = 0;
   for (size_t i = 0; i < image->height; i++)
   {
      int filtType = output[byteCounter++];

#ifdef OILDEBUG_PRINT_SCANLINES
      printf("[OILDEBUG]: Reading scanline #%li, filtType: %i\n", i, filtType);
#endif

      switch (filtType)
      {
         case png_filterType_none:
            break;

         case png_filterType_sub:
            filterData(image, output, bytesPerColor, i, filterSub);
            break;

         case png_filterType_up:
            filterData(image, output, bytesPerColor, i, filterUp);
            break;

         case png_filterType_average:
            filterData(image, output, bytesPerColor, i, filterAverage);
            break;

         case png_filterType_paeth:
            filterData(image, output, bytesPerColor, i, filterPaeth);
            break;

         default:
            OIL_FREE(output);
            oilPushErrorf("[OILERROR]: %i is unknown filter type\n", filtType);
            return 0;
      }

      getImageColors(image, &byteCounter, output, i);
   }

   OIL_FREE(output);
   return 1;
}

int oilProceedChunk(pngImage* image, pngChunk* chunk, int simplified)
{
#ifdef OILDEBUG_PRINT_CHUNK_NAMES
   char *name = oilGetChunkName(chunk);
   printf("[OILDEBUG]: Reading chunk with type: %s\n", name);
   OIL_FREE(name);
#endif

#ifdef OIL_DONT_IGNORE_CHUNKS
   int ancillary = 0;
#else
   int ancillary = (chunk->type & 0xFF) > 'a';
#endif

   if (chunk->type == png_chunk_IHDR)
   {
      image->width = buffToU32(chunk->data);
      image->height = buffToU32(chunk->data + 4);

      image->pixelsInfo->bitDepth = chunk->data[8];
      image->pixelsInfo->usePalette = png_colorflag_palette(chunk->data[9]);
      image->pixelsInfo->useColor = png_colorflag_color(chunk->data[9]);
      image->pixelsInfo->hasAlpha = png_colorflag_alpha(chunk->data[9]);
      image->pixelsInfo->colorFlag = chunk->data[9];

      image->imageData->compression = chunk->data[10];
      image->imageData->filtration = chunk->data[11];
      image->imageData->interlace = chunk->data[12];
   }
   else if (chunk->type == png_chunk_IDAT)
   {
      if (!oilProceedIDAT(image, chunk->data, chunk->length))
      {
         oilPushError("[OILERROR]: Unable to decompress IDAT chunk\n");
         return 0;
      }
   }
   else if (chunk->type == png_chunk_PLTE)
   {
      image->pixelsInfo->paletteLen = chunk->length / 3;
      image->pixelsInfo->palette = OIL_MALLOC(sizeof(oilColor) * image->pixelsInfo->paletteLen);
      for (size_t i = 0; i < image->pixelsInfo->paletteLen; i++)
      {
         image->pixelsInfo->palette[i] = ocolorp(chunk->data[i * 3], chunk->data[i * 3 + 1], chunk->data[i * 3 + 2],
                                                 0xFF);
      }
   }
   else if (chunk->type == png_chunk_gAMA)
   {
      if (simplified) return 1;

      image->pixelsInfo->gammaSet = 1;
      image->pixelsInfo->gamma = buffToU32(chunk->data);
   }
   else if (chunk->type == png_chunk_cHRM)
   {
      if (simplified) return 1;

      image->pixelsInfo->cie = OIL_MALLOC(sizeof(pngCIEInfo));
      image->pixelsInfo->cie->whitePointX = buffToU32(chunk->data);
      image->pixelsInfo->cie->whitePointY = buffToU32(chunk->data + 4);
      image->pixelsInfo->cie->redX = buffToU32(chunk->data + 8);
      image->pixelsInfo->cie->redY = buffToU32(chunk->data + 16);
      image->pixelsInfo->cie->greenX = buffToU32(chunk->data + 20);
      image->pixelsInfo->cie->greenY = buffToU32(chunk->data + 24);
      image->pixelsInfo->cie->blueX = buffToU32(chunk->data + 28);
      image->pixelsInfo->cie->blueY = buffToU32(chunk->data + 32);
   }
   else if (chunk->type == png_chunk_bKGD)
   {
      if (simplified) return 1;

      image->pixelsInfo->bkgColor = ocolorp(0, 0, 0, 0);
      switch (image->pixelsInfo->colorFlag)
      {
         case 0:
         case 4:
            image->pixelsInfo->bkgColor->r = image->pixelsInfo->bkgColor->g = image->pixelsInfo->bkgColor->b =
                    buffToU16(chunk->data);
            break;

         case 2:
         case 6:
            image->pixelsInfo->bkgColor->r = buffToU16(chunk->data);
            image->pixelsInfo->bkgColor->g = buffToU16(chunk->data + 2);
            image->pixelsInfo->bkgColor->b = buffToU16(chunk->data + 4);
            break;

         case 3:
            image->pixelsInfo->bkgColor = image->pixelsInfo->palette[chunk->data[0]];
            break;

         default:
            oilPushErrorf("[OILERROR]: % is unknown colorFlag\n", image->pixelsInfo->colorFlag);
            return 0;
      }

   }
   else if (chunk->type == png_chunk_tEXt)
   {
      if (simplified) return 1;

      /*   if (image->text == NULL)
         {
             image->text = OIL_MALLOC(chunk->length + 1);
             memcpy(image->text, chunk->data, chunk->length);
             image->text[chunk->length] = '\0';
         }
         else
         {
             image->text = realloc(image->text, strlen(image->text) + chunk->length + 2);
             image->text = strcat(strcat(image->text, "\1"), (const char *) chunk->data);
         }*/

      //todo!
   }
   else if (chunk->type == png_chunk_iTXt)
   {
      if (simplified) return 1;

      //todo!
   }

   else if (chunk->type == png_chunk_pHYs)
   {
      if (simplified) return 1;

      image->pixelsInfo->ppuX = buffToU32(chunk->data);
      image->pixelsInfo->ppuY = buffToU32(chunk->data + 4);
      if (chunk->data[8] != 1)
      {
         oilPushErrorf("[OILERROR]: %i is unknown unit\n", chunk->data[8]);
         return 0;
      }
   }
   else if (chunk->type == png_chunk_tIME)
   {
      if (simplified) return 1;

      image->imageData->time = OIL_MALLOC(sizeof(pngTime));
      image->imageData->time->year = buffToU16(chunk->data);
      image->imageData->time->month = chunk->data[2];
      image->imageData->time->day = chunk->data[3];
      image->imageData->time->hour = chunk->data[4];
      image->imageData->time->minute = chunk->data[5];
      image->imageData->time->second = chunk->data[6];
   }
   else
   {
      if (!ancillary)
      {
         char* chunkName = oilGetChunkName(chunk);
         oilPushErrorf("[OILERROR]: %i (or %s) is unknown chunk type\n", chunk->type, chunkName);
         OIL_FREE(chunkName);
         return 0;
      }
   }

   return 1;
}

int oilLoadImage(char* fileName, pngImage** image, int simplified)
{
   FILE* f = fopen(fileName, "rb");

   if (!f)
   {
      oilPushErrorf("[OILERROR]: Unable to open file \"%s\"\n", fileName);
      return 0;
   }

   //Reading and verifying pngImage signature
   uint8_t signBuffer[sizeof(png_signature)];
   if (fread(signBuffer, sizeof(signBuffer), 1, f) != 1)
   {
      oilPushErrorf("[OILERROR]: Unable to read PNG file signature at position %i\n", ftell(f));
   }

   if (memcmp(signBuffer, png_signature, sizeof(png_signature)) != 0)
   {
      oilPushError("[OILERROR]: PNG file signature doesn't match\n");
      return 0;
   }

   pngChunk* chunk = OIL_MALLOC(sizeof(pngChunk));
   chunk->data = NULL;
   *image = oilCreateImg();

   while (1)
   {
      if (fread(buff4, sizeof(buff4), 1, f) != 1)
      {
         oilPushErrorf("[OILERROR]: Unable to read chunk length at position %i\n", ftell(f));
         OIL_FREE(chunk);
         return 0;
      }
      chunk->length = buffToU32(buff4);

      if (fread(&chunk->type, sizeof(chunk->type), 1, f) != 1)
      {
         oilPushErrorf("[OILERROR]: Unable to read chunk type at position %i\n", ftell(f));
         OIL_FREE(chunk);
         oilPNGFreeImage(*image);
         return 0;
      }

      if (chunk->length != 0)
      {
         if(chunk->data) OIL_FREE(chunk->data);

         chunk->data = OIL_MALLOC(sizeof(uint8_t) * chunk->length);
         if (fread(chunk->data, sizeof(uint8_t) * chunk->length, 1, f) != 1)
         {
            oilPushErrorf("[OILERROR]: Unable to read chunk data at position %i\n", ftell(f));
            OIL_FREE(chunk->data);
            OIL_FREE(chunk);
            oilPNGFreeImage(*image);
            return 0;
         }
      }

      if (fread(buff4, sizeof(buff4), 1, f) != 1)
      {
         oilPushErrorf("[OILERROR]: Unable to read chunk crc at position %i\n", ftell(f));
         OIL_FREE(chunk->data);
         OIL_FREE(chunk);
         oilPNGFreeImage(*image);
         return 0;
      }
      chunk->crc = buffToU32(buff4);

      uint32_t crc = CRC32((uint8_t*) &chunk->type, sizeof(chunk->type), 0xFFFFFFFF);
      crc = CRC32(chunk->data, sizeof(uint8_t) * chunk->length, crc);

      if (~crc != chunk->crc)
      {

         oilPushErrorf("[OILERROR]: Chunks crcs doesn't match. Expected %X, but got %X\n", (uint32_t) chunk->crc,
                       (uint32_t) crc);
         OIL_FREE(chunk->data);
         OIL_FREE(chunk);
         oilPNGFreeImage(*image);
         return 0;
      }

      if (chunk->type == png_chunk_IEND)
      {
         break;
      }

      if (!oilProceedChunk(*image, chunk, simplified))
      {
         oilPushError("[OILERROR]: Unable to proceed chunk\n");
         OIL_FREE(chunk->data);
         OIL_FREE(chunk);
         oilPNGFreeImage(*image);
         return 0;
      }
   }


   OIL_FREE(chunk->data);
   OIL_FREE(chunk);

   if (!proceedImageData(*image))
   {
      oilPushError("[OILERROR]: Unable to proceed chunk\n");
      OIL_FREE((*image)->imageData->rawData);
      oilPNGFreeImage(*image);
      return 0;
   }

   OIL_FREE((*image)->imageData->rawData);

   if (fclose(f))
   {
      oilPushError("[OILERROR]: Unable to close file");
      return 0;
   }

   return 1;
}

pngImage* oilPNGLoad(char* fileName, int simplified)
{
   pngImage* img = NULL;
   if (!oilLoadImage(fileName, &img, simplified))
   {
      return NULL;
   }
   else
   {
      return img;
   }
}

void oilPNGFreeImage(pngImage* image)
{
   if (image->colorMatrix) oilColorMatrixFree(image->colorMatrix);
   if (image->pixelsInfo->cie) OIL_FREE(image->pixelsInfo->cie);
   if (image->pixelsInfo->palette)
   {
      for (size_t i = 0; i < image->pixelsInfo->paletteLen; i++)
         OIL_FREE(image->pixelsInfo->palette[i]);
      OIL_FREE(image->pixelsInfo->palette);
   }
   if (image->pixelsInfo->bkgColor) OIL_FREE(image->pixelsInfo->bkgColor);
   OIL_FREE(image->pixelsInfo);

   if (image->imageData->time) OIL_FREE(image->imageData->time);
   if (image->imageData->txtItems) OIL_FREE(image->imageData->txtItems);
   OIL_FREE(image->imageData);

   OIL_FREE(image);
}