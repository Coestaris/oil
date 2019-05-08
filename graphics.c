//
// Created by maxim on 3/18/19.
//

#include "graphics.h"

void writeComponent(uint8_t* data, size_t* byteCounter, uint32_t dataFormat, uint16_t component)
{
    switch(dataFormat)
    {
        default:
        case GL_UNSIGNED_BYTE:
            data[(*byteCounter)++] = (uint8_t)component;
            break;
        case GL_BYTE:
            data[(*byteCounter)++] = (int8_t)component;
            break;
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_SHORT:
            data[(*byteCounter)++] = (uint8_t)component;
            data[(*byteCounter)++] = (uint8_t)(component << 8U);
            break;
        case GL_SHORT:
        case GL_INT:
            data[(*byteCounter)++] = (int8_t)component;
            data[(*byteCounter)++] = (int8_t)(component << 8U);
            break;
    }
}

#define wrComponent(comp) writeComponent(data, byteCounter, dataFormat, comp)

void writeColor(uint8_t* data, size_t* byteCounter, oilColor* color, uint32_t componentFormat, uint32_t dataFormat)
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

        default:
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

imageData* oilGetPNGImageData(pngImage *img, uint32_t componentFormat, uint32_t dataFormat)
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

    imageData* data = malloc(sizeof(imageData));
    data->srcMatrix = img->colorMatrix;
    data->componentFormat = componentFormat;
    data->dataFormat = dataFormat;

    if(componentFormat == GL_AUTO)
    {
        if(!img->pixelsInfo->useColor) {
            componentFormat = GL_RED;
        }
        else
        {
            if (img->pixelsInfo->hasAlpha) componentFormat = GL_RGBA;
            else componentFormat = GL_RGB;
        }
    }

    if(dataFormat == GL_AUTO)
    {
        switch(img->pixelsInfo->bitDepth)
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

    data->dataLen = bytesPerPixel * img->width * img->height;
    data->data = malloc(sizeof(uint8_t) * data->dataLen);

    size_t byteCounter = 0;
    for (uint32_t y = 0; y < img->height; y++) {
        for (uint32_t x = 0; x < img->width; x++) {
            writeColor(data->data, &byteCounter, img->colorMatrix->matrix[y][x], componentFormat, dataFormat);
        }
    }

    return data;
}

void oilFreeImageData(imageData* data)
{
    free(data->data);
    free(data);
}

#define checkGLError(str)  if((error = glGetError()) != GL_NO_ERROR) \
            { \
                oilPushErrorf("[OILERROR]: "str". Gl error: %s (errno %i)\n", gluErrorString(error), error);\
                return 0;\
            }

GLuint oilGetTexture(imageData* data, GLenum wrapping, GLenum magFilter, GLenum minFilter, float* borderColor)
{
    glGetError(); //Reset error flag

    GLenum error;
    GLuint id = 0;

    glGenTextures(1, &id);
    if(((error = glGetError()) != GL_NO_ERROR || id == 0))
    {
        oilPushErrorf("[OILERROR]: Unable to generate texture. Gl error: %s (errno %i)\n", gluErrorString(error), error);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, id);
    checkGLError("Unable to bind texture")

    glTexImage2D(GL_TEXTURE_2D,
            0,
            data->componentFormat,
            data->srcMatrix->width,
            data->srcMatrix->height,
            0,
            data->componentFormat,
            data->dataFormat,
            data->data);
    checkGLError("Unable to fill texture data")
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR   ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST  ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR  ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST ||
       magFilter == GL_LINEAR_MIPMAP_LINEAR   ||
       magFilter == GL_LINEAR_MIPMAP_NEAREST  ||
       magFilter == GL_NEAREST_MIPMAP_LINEAR  ||
       magFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        checkGLError("Unable to generate mipmap")
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    checkGLError("Unable to set tex parameter (wrapping")
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapping);
    checkGLError("Unable to set tex parameter (wrapping")

    if(wrapping == GL_CLAMP_TO_BORDER && borderColor)
    {
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        checkGLError("Unable to set tex parameter (border color")
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    checkGLError("Unable to set tex parameter (min filter)")

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    checkGLError("Unable to set tex parameter (mag filter)")

    glBindTexture(GL_TEXTURE_2D, 0);
    checkGLError("Unable to unbind texture")

    return id;
}

GLuint oilTextureFromPngFileDef(char* fileName, uint32_t componentFormat)
{
    texData data;
    return oilTextureFromPngFile(fileName, componentFormat, 0, &data);
}

GLuint oilTextureFromPngFile(char *filename, uint32_t componentFormat, uint32_t flags, texData* texData)
{
    if(!(flags & OIL_TEX_MIN))          texData->minFilter = OIL_DEFAULT_MIN;
    if(!(flags & OIL_TEX_MAG))          texData->magFilter = OIL_DEFAULT_MAG;
    if(!(flags & OIL_TEX_BORDERCOLOR))  texData->borderColor = OIL_DEFAULT_BORDERCOLOR;
    if(!(flags & OIL_TEX_DATAFRMT))     texData->dataFormat= OIL_DEFAULT_DATAFMT;
    if(!(flags & OIL_TEX_FLIPX))        texData->flipX= OIL_DEFAULT_FLIPX;
    if(!(flags & OIL_TEX_FLIPY))        texData->flipY= OIL_DEFAULT_FLIPY;
    if(!(flags & OIL_TEX_WRAPPING))     texData->wrappingMode = OIL_DEFAULT_WRAPPING;

    pngImage* image;
    if(!(image = oilPNGLoad(filename, 1)))
    {
        oilPushError("[OILERROR]: Unable to load image");
        return 0;
    }

    if(texData->flipX) oilGrFlipX(image->colorMatrix);
    if(texData->flipY) oilGrFlipY(image->colorMatrix);

    imageData* data = oilGetPNGImageData(image, componentFormat, texData->dataFormat);
    if(data == NULL)
    {
        oilPushError("[OILERROR]: Unable to get image data");
        oilPNGFreeImage(image);
        return 0;
    }

    texData->out_width = image->width;
    texData->out_height = image->height;

    GLuint tex = oilGetTexture(data, texData->wrappingMode, texData->magFilter,
            texData->minFilter, texData->borderColor);
    if(tex == 0)
    {
        oilPushError("[OILERROR]: Unable to generate texture");
        oilFreeImageData(data);
        oilPNGFreeImage(image);
        return 0;
    }

    oilFreeImageData(data);
    oilPNGFreeImage(image);

    return tex;
}

void oilGrFill(colorMatrix* matrix, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
    assert(matrix != NULL);
#endif

    for(uint32_t i = 0; i < matrix->height; i++)
        for(uint32_t j = 0; j < matrix->width; j++)
            *matrix->matrix[i][j] = color;
}

void oilGrSetPixel(colorMatrix* matrix, uint32_t x, uint32_t y, oilColor color)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
    assert(matrix != NULL);
    assert(x < matrix->width);
    assert(y < matrix->height);
#endif

    *matrix->matrix[y][x] = color;
}

oilColor oilGrGetPixel(colorMatrix* matrix, uint32_t x, uint32_t y)
{
#ifdef OIL_GRAPHICS_CLIP_CHECKING
    assert(matrix != NULL);
    assert(x < matrix->width);
    assert(y < matrix->height);
#endif

    return *matrix->matrix[y][x];
}

void oilGrFlipX(colorMatrix* matrix)
{
    for(uint32_t i = 0; i < matrix->height; i++)
        for(uint32_t j = 0; j < matrix->width / 2; j++)
        {
            oilColor* tmp = matrix->matrix[i][j];
            matrix->matrix[i][j] = matrix->matrix[i][matrix->width - j - 1];
            matrix->matrix[i][matrix->width - j - 1] = tmp;
        }
}

void oilGrFlipY(colorMatrix* matrix)
{
    for(uint32_t i = 0; i < matrix->height / 2; i++)
        for(uint32_t j = 0; j < matrix->width; j++)
        {
            oilColor* tmp = matrix->matrix[i][j];
            matrix->matrix[i][j] = matrix->matrix[matrix->height - i - 1][j];
            matrix->matrix[matrix->height - i - 1][j] = tmp;
        }

}