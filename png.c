//
// Created by maxim on 3/9/19.
//

#include "png.h"

#define getColorDefaultGetByte data[(*byteCounter)++]
#define colorFlagGetPalette(byte) (uint8_t)((byte & 0b00000001) != 0)
#define colorFlagGetColor(byte) (uint8_t)((byte & 0b00000010) != 0)
#define colorFlagGetAlpha(byte) (uint8_t)((byte & 0b00000100) != 0)

uint8_t buff4[4];

uint32_t buffToU32(const uint8_t *buff)
{
    return (uint32_t) buff[3] |
           (uint32_t) buff[2] << 8 |
           (uint32_t) buff[1] << 16 |
           (uint32_t) buff[0] << 24;
}

uint16_t buffToU16(const uint8_t *buff)
{
    return (uint16_t) buff[1] << 0 |
           (uint16_t) buff[0] << 8;
}

pngImage* oilLoad(char *fileName)
{
    pngImage* img = NULL;
    if(!oilLoadImage(fileName, &img)) {
        return NULL;
    } else {
        return img;
    }
}

int oilLoadImage(char *fileName, pngImage** image)
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
            oilFreeImage(*image);
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
                oilFreeImage(*image);
                return 0;
            }
        }

        if (fread(buff4, sizeof(buff4), 1, f) != 1)
        {
            oilPushErrorf("[OILERROR]: Unable to read chunk crc at position %i\n", ftell(f));
            free(chunk);
            oilFreeImage(*image);
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
            oilFreeImage(*image);
            return 0;
        }

        if (memcmp(&chunk->type, png_chunk_end, sizeof(chunk->type)) == 0)
        {
            break;
        }

        if (!oilProceedChunk(*image, chunk))
        {
            oilPushError("[OILERROR]: Unable to proceed chunk\n");
            free(chunk->data);
            free(chunk);
            oilFreeImage(*image);
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

pngImage *oilCreateImg(void)
{
    pngImage *img = malloc(sizeof(pngImage));
    img->pixelData = malloc(sizeof(pngPixelData));
    img->pixelData->bkgColorSet = 0;
    img->pixelData->cieSet = 0;
    img->pixelData->gammaSet = 0;
    img->txtItems = NULL;
    img->colors = NULL;
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

int oilProceedChunk(pngImage *image, pngChunk *chunk)
{
#ifdef OILDEBUG_PRINT_CHUNK_NAMES
    char *name = oilGetChunkName(chunk);
    printf("[OILDEBUG]: Reading chunk with type: %s\n", name);
    free(name);
#endif

    if (memcmp(&chunk->type, png_chunk_start, sizeof(chunk->type)) == 0)
    {
        image->height = buffToU32(chunk->data);
        image->width = buffToU32(chunk->data + 4);
        image->bitDepth = chunk->data[8];

        image->pixelData->usePalette = colorFlagGetPalette(chunk->data[9]);
        image->pixelData->useColor = colorFlagGetColor(chunk->data[9]);
        image->pixelData->hasAlpha = colorFlagGetAlpha(chunk->data[9]);
        image->colorFlag = chunk->data[9];

        image->compression = chunk->data[10];
        image->filtration = chunk->data[11];
        image->interlace = chunk->data[12];
    }
    else if (memcmp(&chunk->type, png_chunk_gAMA, sizeof(chunk->type)) == 0)
    {

        image->pixelData->gammaSet = 1;
        image->pixelData->gamma = buffToU32(chunk->data);
    }
    else if (memcmp(&chunk->type, png_chunk_cHRM, sizeof(chunk->type)) == 0)
    {
        image->pixelData->cieSet = 1;
        image->pixelData->whitePointX = buffToU32(chunk->data);
        image->pixelData->whitePointY = buffToU32(chunk->data + 4);
        image->pixelData->redX = buffToU32(chunk->data + 8);
        image->pixelData->redY = buffToU32(chunk->data + 16);
        image->pixelData->greenX = buffToU32(chunk->data + 20);
        image->pixelData->greenY = buffToU32(chunk->data + 24);
        image->pixelData->blueX = buffToU32(chunk->data + 28);
        image->pixelData->blueY = buffToU32(chunk->data + 32);
    }
    else if (memcmp(&chunk->type, png_chunk_bKGD, sizeof(chunk->type)) == 0)
    {

        image->pixelData->bkgColorSet = 1;
        switch (image->colorFlag)
        {
            case 0:
            case 4:
                image->pixelData->bkgColor.greyScale = buffToU16(chunk->data);
                break;

            case 2:
            case 6:
                image->pixelData->bkgColor.r = buffToU16(chunk->data);
                image->pixelData->bkgColor.g = buffToU16(chunk->data + 2);
                image->pixelData->bkgColor.b = buffToU16(chunk->data + 4);
                break;

            case 3:
                image->pixelData->bkgColor = image->pixelData->palette[chunk->data[0]];
                break;

            default:
                oilPushErrorf("[OILERROR]: % is unknown colorFlag\n", image->colorFlag);
                return 0;
        }

    }
    else if (memcmp(&chunk->type, png_chunk_tEXt, sizeof(chunk->type)) == 0)
    {

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
    else if (memcmp(&chunk->type, png_chunk_IDAT, sizeof(chunk->type)) == 0)
    {
        if (!oilProceedIDAT(image, chunk->data, chunk->length))
        {
            oilPushError("[OILERROR]: Unable to decompress IDAT chunk\n");
            return 0;
        }
    }
    else if (memcmp(&chunk->type, png_chunk_PLTE, sizeof(chunk->type)) == 0)
    {
        image->pixelData->paletteLen = chunk->length / 3;
        image->pixelData->palette = malloc(sizeof(pngColor) * image->pixelData->paletteLen);
        for (size_t i = 0; i < image->pixelData->paletteLen; i++)
        {
            image->pixelData->palette[i].r = chunk->data[i * 3];
            image->pixelData->palette[i].g = chunk->data[i * 3 + 1];
            image->pixelData->palette[i].b = chunk->data[i * 3 + 2];
        }
    }
    else
    {
        char* chunkName = oilGetChunkName(chunk);
        oilPushErrorf("[OILERROR]: %i (or %s) is unknown chunk type\n", chunk->type, chunkName);
        free(chunkName);
        return 0;
    }

    return 1;
}

void oilColorMatrixAlloc(pngImage* image, uint8_t allocColors)
{
    image->colors = malloc(sizeof(pngColor**) * image->height);
    for(uint32_t i = 0; i < image->height; i++)
    {
        image->colors[i] = malloc((sizeof(pngColor*) * image->width));
        if(allocColors)
        {
            for (uint32_t j = 0; j < image->height; j++)
            {
                image->colors[i][j] = malloc(sizeof(pngColor));
            }
        }
    }
}

void oilColorMatrixFree(pngImage* image)
{
    for(uint32_t i = 0; i < image->height; i++)
    {
        for(uint32_t j = 0; j < image->height; j++)
            free(image->colors[i][j]);
        free(image->colors[i]);
    }
    free(image->colors);
}

void oilFreeImage(pngImage* image)
{
    if(image->colors) oilColorMatrixFree(image);
    free(image->pixelData);
    free(image->txtItems);
    free(image);
}

void printColor(pngColor* color, uint32_t flag, uint8_t hex)
{
    if (colorFlagGetAlpha(flag))
    {
        if (colorFlagGetColor(flag))
        {
            if(hex)
                printf("RGBA(%.2X, %.2X, %.2X, %.2X)", color->r, color->g, color->b, color->a);
            else
                printf("RGBA(%.3i, %.3i, %.3i, %.3i)", color->r, color->g, color->b, color->a);
        }
        else
        {
            if(hex)
                printf("Grayscale(%.2X, %.2X)", color->greyScale, color->a);
            else
                printf("Grayscale(%.3i, %.3i)", color->greyScale, color->a);
        }
    }
    else
    {
        if (colorFlagGetColor(flag))
        {
            if(hex)
                printf("RGB(%.2X, %.2X, %.2X)", color->r, color->g, color->b);
            else
                printf("RGB(%.3i, %.3i, %.3i)", color->r, color->g, color->b);
        }
        else
        {
            if(hex)
                printf("Grayscale(%.2X)", color->greyScale);
            else
                printf("Grayscale(%.3i)", color->greyScale);
        }
    }
}

void getImageColors(pngImage *image, size_t *byteCounter, uint8_t *data, size_t scanlineIndex)
{
    for (size_t i = 0; i < image->width; i++)
    {
        pngColor* color = image->colors[scanlineIndex][i];

        if (image->pixelData->usePalette)
        {
            color = &image->pixelData->palette[getColorDefaultGetByte << 8 | getColorDefaultGetByte];
        }
        else
        {
            if (image->pixelData->useColor)
            {
                if (image->bitDepth == 16)
                {
                    color->r = getColorDefaultGetByte << 8 | getColorDefaultGetByte;
                    color->g = getColorDefaultGetByte << 8 | getColorDefaultGetByte;
                    color->b = getColorDefaultGetByte << 8 | getColorDefaultGetByte;
                }
                else
                {
                    color->r = getColorDefaultGetByte;
                    color->g = getColorDefaultGetByte;
                    color->b = getColorDefaultGetByte;
                }
            }
            else
            {
                if (image->bitDepth == 16)
                {
                    color->greyScale = getColorDefaultGetByte << 8 | getColorDefaultGetByte;
                }
                else
                {
                    color->greyScale = getColorDefaultGetByte;
                }
            }
        }

        if (image->pixelData->hasAlpha)
        {
            if (image->bitDepth == 16)
            {
                color->a = getColorDefaultGetByte << 8 | getColorDefaultGetByte;
            }
            else
            {
                color->a = getColorDefaultGetByte;
            }
        }

        image->colors[scanlineIndex][i] = color;
    }
}

int oilProceedIDAT(pngImage *image, uint8_t *data, size_t length)
{

#ifdef OILDEBUG_PRINT_COMPRESSED_DATA
    puts("[OILDEBUG]: Compressed image data:");
    for(int i = 0; i < length; i++) {
        printf("%.2X ", data[i]);
        if((i + 1) % 30 == 0) putchar('\n');
    }
    putchar('\n');
#endif

    uint8_t bytesPerColor = 0;
    if (image->pixelData->usePalette)
    {
        bytesPerColor = 1;
    }
    else if (image->pixelData->useColor)
    {
        bytesPerColor = (uint8_t) (3 * image->bitDepth / 8);
    }
    else
    {
        bytesPerColor = (uint8_t) (image->bitDepth / 8);
    }

    if (image->pixelData->hasAlpha)
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
        puts(zError(result));
        oilPushError("[OILERROR]: Unable to decompress data\n");
        return 0;
    }

    inflateEnd(&infstream);

#ifdef OILDEBUG_PRINT_DECOMPRESSED_DATA
    puts("[OILDEBUG]: Decompressed image data:");
    for(int i = 0; i < outputLen; i++) {
        printf("%.2X ", output[i]);
        if((i + 1) % 30 == 0) putchar('\n');
    }
    putchar('\n');
#endif

    oilColorMatrixAlloc(image, 1);

    size_t byteCounter = 0;
    for (size_t i = 0; i < image->height; i++)
    {
        int filtType = output[byteCounter++];

#ifdef OILDEBUG_PRINT_SCANLINES
        printf("[OILDEBUG]: Reading scanline #%li, filtType: %i\n", i, filtType);
#endif

        switch (filtType)
        {
            case 0:
                //None
                getImageColors(image, &byteCounter, output, i);
                break;

            case 1:
                //Sub

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

            case 2:
                //Up
            case 3:
                //Average
            case 4:
                //Paeth
            default:
                free(output);
                oilPushErrorf("[OILERROR]: %i is unknown filter type\n", filtType);
                return 0;
        }
    }

    free(output);
    return 1;
}