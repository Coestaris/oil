//
// Created by maxim on 3/9/19.
//

#include "png.h"

uint8_t buff4[4];

uint32_t buffToU32(const uint8_t *buff)
{
    return (uint32_t) buff[3]       |
           (uint32_t) buff[2] << 8  |
           (uint32_t) buff[1] << 16 |
           (uint32_t) buff[0] << 24;
}

uint16_t buffToU16(const uint8_t *buff)
{
    return (uint16_t) buff[1] << 0  |
           (uint16_t) buff[0] << 8;
}

pngImage *oilCreateImg(void)
{
    pngImage *img = malloc(sizeof(pngImage));
    img->pixelsInfo = malloc(sizeof(pngPixelData));
    img->pixelsInfo->bkgColorSet = 0;
    img->pixelsInfo->ppuY = 0;
    img->pixelsInfo->ppuX = 0;
    img->pixelsInfo->cieSet = 0;
    img->pixelsInfo->gammaSet = 0;
    img->txtItems = NULL;
    img->colorMatrix = NULL;
    return img;
}

char *oilGetChunkName(pngChunk *chunk)
{
    char *str = malloc(5);
    for (size_t i = 0; i < 4; i++)
    {
        str[i] = (char) ((chunk->type >> i * 8) & 0xFF);
    }
    str[4] = '\0';
    return str;
}


void getImageColors(pngImage *image, size_t *byteCounter, uint8_t *data, size_t scanlineIndex)
{
    for (size_t i = 0; i < image->width; i++)
    {
        oilColor* color = image->colorMatrix->matrix[scanlineIndex][i];

        if (image->pixelsInfo->usePalette)
        {
            color = &image->pixelsInfo->palette[png_get_next_byte];
        }
        else
        {
            if (image->pixelsInfo->useColor)
            {
                if (image->bitDepth == 16)
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
                if (image->bitDepth == 16)
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
            if (image->bitDepth == 16)
            {
                color->a = png_get_next_byte << 8 | png_get_next_byte;
            }
            else
            {
                color->a = png_get_next_byte;
            }
        }

        image->colorMatrix->matrix[scanlineIndex][i] = color;
    }
}

int oilProceedIDAT(pngImage *image, uint8_t *data, size_t length)
{

#ifdef OILDEBUG_PRINT_COMPRESSED_DATA
    printf("[OILDEBUG]: Compressed image data (%li bytes):\n", length);
    for(int i = 0; i < length; i++) {
        printf("%.2X ", data[i]);
        if((i + 1) % 30 == 0) putchar('\n');
    }
    putchar('\n');
#endif

    uint8_t bytesPerColor = 0;
    if (image->pixelsInfo->usePalette)
    {
        bytesPerColor = 1;
    }
    else if (image->pixelsInfo->useColor)
    {
        bytesPerColor = (uint8_t) (3 * image->bitDepth / 8);
    }
    else
    {
        bytesPerColor = (uint8_t) (image->bitDepth / 8);
    }

    if (image->pixelsInfo->hasAlpha)
    {
        bytesPerColor += image->bitDepth / 8;
    }

    size_t outputLen = bytesPerColor * image->width * image->height + image->height - 1;

    uint8_t *output = malloc(outputLen);
    memset(output, 0, outputLen);

    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;

    infstream.avail_in = (uInt) length;
    infstream.next_in = data;
    infstream.avail_out = (uInt) outputLen;
    infstream.next_out = output;

    inflateInit(&infstream);

    int result;
    if ((result = inflate(&infstream, Z_NO_FLUSH)) != Z_OK)
    {
        free(output);
        oilPushErrorf("[OILERROR]: Unable to decompress data. ZLIB error: %s\n", zError(result));
        inflateEnd(&infstream);
        return 0;
    }

    inflateEnd(&infstream);

#ifdef OILDEBUG_PRINT_DECOMPRESSED_DATA
    printf("[OILDEBUG]: Decompressed image data (%li bytes):\n", outputLen);
    for(int i = 0; i < outputLen; i++) {
        printf("%.2X ", output[i]);
        if((i + 1) % 30 == 0) putchar('\n');
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
                getImageColors(image, &byteCounter, output, i);
                break;

            case png_filterType_sub:
                for (size_t component = 1; component < image->width; component++)
                {
                    for (size_t byte = 0; byte < bytesPerColor; byte++)
                    {
                        output[byteCounter + component * bytesPerColor + byte] =
                                output[byteCounter + component * bytesPerColor + byte] +
                                output[byteCounter + (component - 1) * bytesPerColor + byte];
                    }
                }
                getImageColors(image, &byteCounter, output, i);
                break;

            case png_filterType_up:
            case png_filterType_average:
            case png_filterType_paeth:
            default:
                free(output);
                oilPushErrorf("[OILERROR]: %i is unknown filter type\n", filtType);
                return 0;
        }
    }

    free(output);
    return 1;
}

int oilProceedChunk(pngImage *image, pngChunk *chunk, int simplified)
{
#ifdef OILDEBUG_PRINT_CHUNK_NAMES
    char *name = oilGetChunkName(chunk);
    printf("[OILDEBUG]: Reading chunk with type: %s\n", name);
    free(name);
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
        image->bitDepth = chunk->data[8];

        image->pixelsInfo->usePalette = png_colorflag_palette(chunk->data[9]);
        image->pixelsInfo->useColor = png_colorflag_color(chunk->data[9]);
        image->pixelsInfo->hasAlpha = png_colorflag_alpha(chunk->data[9]);
        image->colorFlag = chunk->data[9];

        image->compression = chunk->data[10];
        image->filtration = chunk->data[11];
        image->interlace = chunk->data[12];
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
        image->pixelsInfo->palette = malloc(sizeof(oilColor) * image->pixelsInfo->paletteLen);
        for (size_t i = 0; i < image->pixelsInfo->paletteLen; i++)
        {
            image->pixelsInfo->palette[i].r = chunk->data[i * 3];
            image->pixelsInfo->palette[i].g = chunk->data[i * 3 + 1];
            image->pixelsInfo->palette[i].b = chunk->data[i * 3 + 2];
        }
    }
    else if (chunk->type == png_chunk_gAMA)
    {
        if(simplified) return 1;

        image->pixelsInfo->gammaSet = 1;
        image->pixelsInfo->gamma = buffToU32(chunk->data);
    }
    else if (chunk->type == png_chunk_cHRM)
    {
        if(simplified) return 1;

        image->pixelsInfo->cieSet = 1;
        image->pixelsInfo->whitePointX = buffToU32(chunk->data);
        image->pixelsInfo->whitePointY = buffToU32(chunk->data + 4);
        image->pixelsInfo->redX = buffToU32(chunk->data + 8);
        image->pixelsInfo->redY = buffToU32(chunk->data + 16);
        image->pixelsInfo->greenX = buffToU32(chunk->data + 20);
        image->pixelsInfo->greenY = buffToU32(chunk->data + 24);
        image->pixelsInfo->blueX = buffToU32(chunk->data + 28);
        image->pixelsInfo->blueY = buffToU32(chunk->data + 32);
    }
    else if (chunk->type == png_chunk_bKGD)
    {
        if(simplified) return 1;

        image->pixelsInfo->bkgColorSet = 1;
        switch (image->colorFlag)
        {
            case 0:
            case 4:
                image->pixelsInfo->bkgColor.r =
                  image->pixelsInfo->bkgColor.g =
                  image->pixelsInfo->bkgColor.b = buffToU16(chunk->data);
                break;

            case 2:
            case 6:
                image->pixelsInfo->bkgColor.r = buffToU16(chunk->data);
                image->pixelsInfo->bkgColor.g = buffToU16(chunk->data + 2);
                image->pixelsInfo->bkgColor.b = buffToU16(chunk->data + 4);
                break;

            case 3:
                image->pixelsInfo->bkgColor = image->pixelsInfo->palette[chunk->data[0]];
                break;

            default:
                oilPushErrorf("[OILERROR]: % is unknown colorFlag\n", image->colorFlag);
                return 0;
        }

    }
    else if (chunk->type == png_chunk_tEXt)
    {
        if(simplified) return 1;

        /*   if (image->text == NULL)
           {
               image->text = malloc(chunk->length + 1);
               memcpy(image->text, chunk->data, chunk->length);
               image->text[chunk->length] = '\0';
           }
           else
           {
               image->text = realloc(image->text, strlen(image->text) + chunk->length + 2);
               image->text = strcat(strcat(image->text, "\1"), (const char *) chunk->data);
           }*/
    }
    else if (chunk->type == png_chunk_pHYs)
    {
        if(simplified) return 1;

        image->pixelsInfo->ppuX = buffToU32(chunk->data);
        image->pixelsInfo->ppuY = buffToU32(chunk->data + 4);
        if(chunk->data[8] != 1)
        {
            oilPushErrorf("[OILERROR]: %i is unknown unit\n", chunk->data[8]);
            return 0;
        }
    }
    else
    {
        if (!ancillary)
        {
            char *chunkName = oilGetChunkName(chunk);
            oilPushErrorf("[OILERROR]: %i (or %s) is unknown chunk type\n", chunk->type, chunkName);
            free(chunkName);
            return 0;
        }
    }

    return 1;
}

int oilLoadImage(char *fileName, pngImage** image, int simplified)
{
    FILE *f = fopen(fileName, "rb");

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

    pngChunk *chunk = malloc(sizeof(pngChunk));
    *image = oilCreateImg();

    while (1)
    {

        if (fread(buff4, sizeof(buff4), 1, f) != 1)
        {
            oilPushErrorf("[OILERROR]: Unable to read chunk length at position %i\n", ftell(f));
            free(chunk);
            return 0;
        }
        chunk->length = buffToU32(buff4);

        if (fread(&chunk->type, sizeof(chunk->type), 1, f) != 1)
        {
            oilPushErrorf("[OILERROR]: Unable to read chunk type at position %i\n", ftell(f));
            free(chunk);
            oilPNGFreeImage(*image);
            return 0;
        }

        if (chunk->length != 0)
        {
            chunk->data = malloc(sizeof(uint8_t) * chunk->length);
            if (fread(chunk->data, sizeof(uint8_t) * chunk->length, 1, f) != 1)
            {
                oilPushErrorf("[OILERROR]: Unable to read chunk data at position %i\n", ftell(f));
                free(chunk->data);
                free(chunk);
                oilPNGFreeImage(*image);
                return 0;
            }
        }

        if (fread(buff4, sizeof(buff4), 1, f) != 1)
        {
            oilPushErrorf("[OILERROR]: Unable to read chunk crc at position %i\n", ftell(f));
            free(chunk);
            oilPNGFreeImage(*image);
            return 0;
        }
        chunk->crc = buffToU32(buff4);

        uint32_t crc = CRC32((uint8_t *) &chunk->type, sizeof(chunk->type), 0xFFFFFFFF);
        crc = CRC32(chunk->data, sizeof(uint8_t) * chunk->length, crc);

        if (~crc != chunk->crc)
        {

            oilPushErrorf("[OILERROR]: Chunks crcs doesn't match. Expected %X, but got %X\n", (uint32_t) chunk->crc, (uint32_t) crc);
            free(chunk->data);
            free(chunk);
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
            free(chunk->data);
            free(chunk);
            oilPNGFreeImage(*image);
            return 0;
        }
    }

    free(chunk);

    if (fclose(f))
    {
        oilPushError("[OILERROR]: Unable to close file");
        return 0;
    }

    return 1;
}

pngImage* oilPNGLoad(char *fileName, int simplified)
{
    pngImage* img = NULL;
    if(!oilLoadImage(fileName, &img, simplified)) {
        return NULL;
    } else {
        return img;
    }
}


void oilPNGFreeImage(pngImage *image)
{
    if(image->colorMatrix) oilColorMatrixFree(image->colorMatrix);
    free(image->pixelsInfo);
    free(image->txtItems);
    free(image);
}