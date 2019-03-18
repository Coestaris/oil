//
// Created by maxim on 3/18/19.
//

#include "ogl.h"

void writeComponent(uint8_t* data, size_t* byteCounter, uint32_t dataFormat, uint16_t component)
{
    switch(dataFormat)
    {
        case GL_UNSIGNED_BYTE:
            data[(*byteCounter)++] = (uint8_t)component;
            break;
        case GL_BYTE:
            data[(*byteCounter)++] = (int8_t)component;
            break;
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_SHORT:
            data[(*byteCounter)++] = (uint8_t)component;
            data[(*byteCounter)++] = (uint8_t)(component << 8);
            break;
        case GL_SHORT:
        case GL_INT:
            data[(*byteCounter)++] = (int8_t)component;
            data[(*byteCounter)++] = (int8_t)(component << 8);
            break;
    }
}

#define wrComponent(comp) writeComponent(data, byteCounter, dataFormat, comp)

void writeColor(uint8_t* data, size_t* byteCounter, pngColor* color, uint32_t componentFormat, uint32_t dataFormat)
{
    switch(componentFormat)
    {
        case GL_RED:
            wrComponent(color->r);
            break;

        case GL_RG:
            wrComponent(color->r);
            wrComponent(color->g);
            break;

        case GL_RGB:
            wrComponent(color->r);
            wrComponent(color->g);
            wrComponent(color->b);
            break;

        case GL_BGR:
            wrComponent(color->b);
            wrComponent(color->g);
            wrComponent(color->r);
            break;

        case GL_RGBA:
            wrComponent(color->r);
            wrComponent(color->g);
            wrComponent(color->b);
            wrComponent(color->a);
            break;

        case GL_BGRA:
            wrComponent(color->b);
            wrComponent(color->g);
            wrComponent(color->r);
            wrComponent(color->a);
            break;
    }
}

pngImageData* oilGetImageData(pngImage* image, uint32_t componentFormat, uint32_t dataFormat)
{
    /* Allowed componentFormats
     * - GL_RED,
     * - GL_RG,
     * - GL_RGB,
     * - GL_BGR,
     * - GL_RGBA,
     * - GL_BGRA
     * - GL_AUTO
     *
     * Allowed dataFormats
     *  - GL_UNSIGNED_BYTE,
     *  - GL_BYTE,
     *  - GL_UNSIGNED_SHORT,
     *  - GL_SHORT,
     *  - GL_UNSIGNED_INT,
     *  - GL_INT
     *  - GL_AUTO
     */

    if(componentFormat != GL_RED && componentFormat != GL_RG && componentFormat != GL_RGB &&
       componentFormat != GL_BGR && componentFormat != GL_BGRA && componentFormat != GL_RGBA &&
       componentFormat != GL_AUTO)
    {
        oilPushError("[OILERROR]: Unknown component format\n");
        return NULL;
    }

    if(dataFormat != GL_UNSIGNED_BYTE && dataFormat != GL_BYTE && dataFormat != GL_UNSIGNED_SHORT &&
       dataFormat != GL_SHORT && dataFormat != GL_UNSIGNED_INT && dataFormat != GL_INT &&
       dataFormat != GL_AUTO)
    {
        oilPushError("[OILERROR]: Unknown data format\n");
        return NULL;
    }

    pngImageData* data = malloc(sizeof(pngImageData));
    data->srcImage = image;
    data->componentFormat = componentFormat;
    data->dataFormat = dataFormat;

    if(componentFormat == GL_AUTO)
    {
        if(!image->pixelsInfo->useColor) {
            componentFormat = GL_RED;
        }
        else
        {
            if (image->pixelsInfo->hasAlpha) componentFormat = GL_RGBA;
            else componentFormat = GL_RGB;
        }
    }

    if(dataFormat == GL_AUTO)
    {
        switch(image->bitDepth)
        {
            case 1:
            case 2:
            case 4:
            case 8:
                dataFormat = GL_UNSIGNED_BYTE;
                break;
            case 16:
                dataFormat = GL_UNSIGNED_SHORT;
                break;
            default:
                oilPushError("[OILERROR]: When using GL_AUTO bitdepth must be valid value\n");
                free(data);
                return NULL;
        }
    }

    uint8_t bytesPerPixel = 0;
    switch(componentFormat)
    {
        case GL_RED:
            bytesPerPixel = 1;
            break;
        case GL_RG:
            bytesPerPixel = 2;
            break;
        case GL_RGB:
        case GL_BGR:
            bytesPerPixel = 3;
            break;
        case GL_RGBA:
        case GL_BGRA:
            bytesPerPixel = 4;
            break;
        default:
            break;
    }

    switch(dataFormat)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            bytesPerPixel *= 1;
            break;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
            bytesPerPixel *= 2;
            break;
        case GL_UNSIGNED_INT:
        case GL_INT:
            bytesPerPixel *= 4;
            break;
        default:
            break;
    }

    data->dataLen = bytesPerPixel * image->width * image->height;
    data->data = malloc(sizeof(uint8_t) * data->dataLen);

    size_t byteCounter = 0;
    for (uint32_t y = 0; y < image->height; y++) {
        for (uint32_t x = 0; x < image->width; x++) {
            writeColor(data->data, &byteCounter, image->colors[y][x], componentFormat, dataFormat);
        }
    }

    return data;
}

void oilFreeImageData(pngImageData* data)
{
    free(data->data);
    free(data);
}

GLuint oilGetTexture(pngImageData* data)
{
    GLenum error;
    GLuint id = 0;

    glGenTextures(1, &id);
    if((error = glGetError()) != GL_NO_ERROR || id == 0)
    {
        oilPushErrorf("[OILERROR]: Unable to generate texture. Gl error: %s (errno %i)\n", gluErrorString(error), error);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, id);
    if((error = glGetError()) != GL_NO_ERROR)
    {
        oilPushErrorf("[OILERROR]: Unable to bind texture. Gl error: %s (errno %i)\n", gluErrorString(error), error);
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D,
            0,
            data->componentFormat,
            data->srcImage->width,
            data->srcImage->height,
            0,
            data->componentFormat,
            data->dataFormat,
            data->data);
    if((error = glGetError()) != GL_NO_ERROR)
    {
        oilPushErrorf("[OILERROR]: Unable fill texture with data. Gl error: %s (errno %i)\n", gluErrorString(error), error);
        return 0;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return 1;
}

GLuint oilTextureFromFile(char* filename, uint32_t componentFormat, uint32_t dataFormat)
{
    pngImage* image;
    if(!(image = oilLoad(filename)))
    {
        oilPushError("[OILERROR]: Unable to load image");
        return 0;
    }

    pngImageData* data = oilGetImageData(image, componentFormat, dataFormat);
    if(data == NULL)
    {
        oilPushError("[OILERROR]: Unable to get image data");
        oilFreeImage(image);
        return 0;
    }

    GLuint tex = oilGetTexture(data);
    if(tex == 0)
    {
        oilPushError("[OILERROR]: Unable to generate texture");
        oilFreeImageData(data);
        oilFreeImage(image);
        return 0;
    }

    oilFreeImageData(data);
    oilFreeImage(image);

    return tex;
}