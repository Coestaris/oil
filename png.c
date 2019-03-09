//
// Created by maxim on 3/9/19.
//

#include "png.h"

uint8_t buff4[4];

uint32_t buffToU32(const uint8_t* buff)
{
    return (uint32_t)buff[3]       |
           (uint32_t)buff[2] << 8  |
           (uint32_t)buff[1] << 16 |
           (uint32_t)buff[0] << 24;
}

uint16_t buffToU16(const uint8_t* buff)
{
    return (uint16_t)buff[1] << 0 |
           (uint16_t)buff[0] << 8;
}

int oilGetChunks(char* fileName)
{
    FILE* f = fopen(fileName, "rb");

    if(!f) {
        oilPushErrorf("Unable to open file \"%s\"", fileName);
        return 0;
    }

    //Reading and verifying pngimage signature
    uint8_t signBuffer[sizeof(png_signature)];
    if (fread(signBuffer, sizeof(signBuffer), 1, f) != 1) {
        oilPushErrorf("Unable to read PNG file signature at position %i", ftell(f));
    }

    if( memcmp(signBuffer, png_signature, sizeof(png_signature)) != 0 ) {
        oilPushError("PNG file signature doesn't match");
        return 0;
    }

    pngchunk* chunk  = malloc(sizeof(pngchunk));
    pngimage* img = oilCreateImg();

    while(1) {

        if (fread(buff4, sizeof(buff4), 1, f) != 1) {
            oilPushErrorf("Unable to read chunk length at position %i", ftell(f));
            free(chunk);
            return 0;
        }
        chunk->length = buffToU32(buff4);

        if (fread(&chunk->type, sizeof(chunk->type), 1, f) != 1) {
            oilPushErrorf("Unable to read chunk type at position %i", ftell(f));
            free(chunk);
            free(img);
            return 0;
        }

        if(chunk->length != 0) {
            chunk->data = malloc(sizeof(uint8_t) * chunk->length);
            if (fread(chunk->data, sizeof(uint8_t) * chunk->length, 1, f) != 1) {
                oilPushErrorf("Unable to read chunk data at position %i", ftell(f));
                free(chunk->data);
                free(chunk);
                free(img);
                return 0;
            }
        }

        if (fread(buff4, sizeof(buff4), 1, f) != 1) {
            oilPushErrorf("Unable to read chunk crc at position %i", ftell(f));
            free(chunk);
            free(img);
            return 0;
        }
        chunk->crc = buffToU32(buff4);

        uint32_t crc = CRC32((uint8_t* )&chunk->type, sizeof(chunk->type), 0xFFFFFFFF);
        crc = CRC32(chunk->data, sizeof(uint8_t) * chunk->length, crc);

        if(~crc != chunk->crc) {

            oilPushErrorf("Chunks crcs doesn't match. Expected %i, but got %i", chunk->crc, crc);
            free(chunk->data);
            free(chunk);
            free(img);
            return 0;
        }

        if(memcmp(&chunk->type, png_chunk_end, sizeof(chunk->type)) == 0) {
            break;
        }

        if(!oilProceedChunk(img, chunk)) {
            oilPushError("Unable to proceed chunk");
            free(chunk->data);
            free(chunk);
            free(img);
            return 0;
        }
    }

    free(chunk);

    if( fclose(f) ) {
        oilPushError("Unable to close file");
        return 0;
    }

    return 1;
}

pngimage* oilCreateImg(void)
{
    pngimage* img = malloc(sizeof(pngimage));
    img->color_management.bkgColorSet = 0;
    img->color_management.cieSet = 0;
    img->color_management.gammaSet = 0;
    img->text = NULL;
    return img;
}

char* oilGetChunkName(pngchunk* chunk)
{
    char* str = malloc(5);
    for(size_t i = 0; i < 4; i++) {
        str[i] = (char)((chunk->type >> i * 8) & 0xFF);
    }
    str[4] = '\0';
    return str;
}

int oilProceedChunk(pngimage* image, pngchunk* chunk)
{
    char* name = oilGetChunkName(chunk);
    puts(name);
    free(name);

    if(memcmp(&chunk->type, png_chunk_start, sizeof(chunk->type)) == 0) {
        image->height = buffToU32(chunk->data);
        image->width = buffToU32(chunk->data + 4);
        image->bit_depth = chunk->data[8];

        image->color_management.usePalette = (uint8_t)((chunk->data[9] & 0b00000001) != 0);
        image->color_management.useColor = (uint8_t)((chunk->data[9] & 0b00000010) != 0);
        image->color_management.hasAlpha = (uint8_t)((chunk->data[9] & 0b00000100) != 0);
        image->colorFlag = chunk->data[9];

        image->compression = chunk->data[10];
        image->filtration = chunk->data[11];
        image->interlace = chunk->data[12];
    }

    else if(memcmp(&chunk->type, png_chunk_gAMA, sizeof(chunk->type)) == 0) {

        image->color_management.gammaSet = 1;
        image->color_management.gamma = buffToU32(chunk->data);
    }

    else if(memcmp(&chunk->type, png_chunk_cHRM, sizeof(chunk->type)) == 0) {

        image->color_management.cieSet = 1;
        image->color_management.whitePointX = buffToU32(chunk->data);
        image->color_management.whitePointY = buffToU32(chunk->data + 4);
        image->color_management.redX        = buffToU32(chunk->data + 8);
        image->color_management.redY        = buffToU32(chunk->data + 16);
        image->color_management.greenX      = buffToU32(chunk->data + 20);
        image->color_management.greenY      = buffToU32(chunk->data + 24);
        image->color_management.blueX       = buffToU32(chunk->data + 28);
        image->color_management.blueY       = buffToU32(chunk->data + 32);
    }
    else if(memcmp(&chunk->type, png_chunk_bKGD, sizeof(chunk->type)) == 0) {

        image->color_management.bkgColorSet = 1;
        switch(image->colorFlag) {
            case 0:
            case 4:
                image->color_management.bkgColor.greyScale = buffToU16(chunk->data);
                break;

            case 2:
            case 6:
                image->color_management.bkgColor.r = buffToU16(chunk->data);
                image->color_management.bkgColor.g = buffToU16(chunk->data + 2);
                image->color_management.bkgColor.b = buffToU16(chunk->data + 4);
                break;

            case 3:
                image->color_management.bkgColor.paletteIndex = chunk->data[0];
                break;

            default:
                oilPushError("Unknown colorFlag\n");
                return 0;
        }

    }
    else if(memcmp(&chunk->type, png_chunk_tEXt, sizeof(chunk->type)) == 0) {

        if(image->text == NULL) {
            image->text = malloc(chunk->length + 1);
            memcpy(image->text, chunk->data, chunk->length);
            image->text[chunk->length] = '\0';
        } else {

            image->text = realloc(image->text, strlen(image->text) + chunk->length + 2);
            image->text = strcat(strcat(image->text, "\1"), (const char *) chunk->data);
        }
    }
    else if(memcmp(&chunk->type, png_chunk_IDAT, sizeof(chunk->type)) == 0) {

    }

    else
    {
        oilPushError("Unknown chunk type =c\n");
        return 0;
    }

    return 1;
}