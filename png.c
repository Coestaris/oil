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

            oilPushErrorf("Chunks crcs doesn't match. Expected %X, but got %X", (uint32_t)chunk->crc, (uint32_t)crc);
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
        if(!oilProceedIDAT(image, chunk->data, chunk->length)) {
            oilPushError("Unable to decompress IDAT chunk =c\n");
            return 0;
        }
    }

    else
    {
        oilPushError("Unknown chunk type =c\n");
        return 0;
    }

    return 1;
}

zlib_header* getZlibHeader(uint8_t* data, size_t* offset)
{
    zlib_header* header = malloc(sizeof(zlib_header));

    header->compMethod = (uint8_t)(data[0] & 0b00001111);
    header->compInfo =   (uint8_t)(data[0] & 0b11110000) >> 4;

    if(header->compMethod != 8) {
        oilPushErrorf("%i is unknown compMethod, it can be only 8\n", header->compMethod);
        return NULL;
    }

    header->fCheck = (uint8_t)(data[1] & 0b00011111);
    header->fDict  = (uint8_t)(data[1] & 0b00100000) >> 5;
    header->fLevel = (uint8_t)(data[1] & 0b11000000) >> 6;

    if( ((data[0] << 8) | data[1]) % 31 != 0 ) {
        oilPushErrorf("fCheck error. Expected %i %% 31 to be equal 0\n", ((data[0] << 8) | data[1]));
        return NULL;
    }

    if(header->fDict) {
        header->dict = buffToU32(data + 2);
        *offset = 6;
    } else {
        *offset = 2;
    }

    return header;
}

int oilProceedIDAT(pngimage* image, uint8_t * data, size_t length)
{
    for(size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    putchar('\n');

    size_t offset = 0;
    zlib_header* header = getZlibHeader(data, &offset);
    if(header == NULL) {
        oilPushError("Unable to parse ZLIb dict\n");
        return 0;
    }

    return 1;
}
//63 F8 3F 93 E1 3F 03 C3 CC FF 20 1A C8 00 22 24 0E 58 12 85 33 D3 F8 3F 03 32 07 44 03 00 AA 05 23 77