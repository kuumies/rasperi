/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::MipmapGenerator class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/vec2.hpp>
#include <QtGui/QImage>
#include "rasperi_texture_2d.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
   Calculates an average of four pixel values.
 * ---------------------------------------------------------------- */
template<typename T, int C>
std::array<T, C> average(const glm::ivec2 p00,
                         const glm::ivec2 p10,
                         const glm::ivec2 p01,
                         const glm::ivec2 p11,
                         const int width,
                         const std::vector<T>& src)
{
    std::array<T, C> out;
    for (int i = 0; i < C; ++i)
    {
        T v00 = src[p00.y * width * C + p00.x * C + i];
        T v10 = src[p10.y * width * C + p10.x * C + i];
        T v01 = src[p01.y * width * C + p01.x * C + i];
        T v11 = src[p11.y * width * C + p11.x * C + i];
        out[i] = (v00 + v10 + v01 + v11) / T(4);
    }
    return out;
}

/* ---------------------------------------------------------------- *
   Minify scaling with box filter for power-of-two images sizes
   See https://www.compuphase.com/graphic/scale2.htm
 * ---------------------------------------------------------------- */
template<typename T, int C>
void scaleMinify(int srcWidth, int srcHeight,
                 const std::vector<T>& src,
                 std::vector<T>& dst)
{
    const int dstWidth  = srcWidth  / 2;
    const int dstHeight = srcHeight / 2;

    // Resize the destination if necessary.
    const int size = dstWidth * dstHeight * C;
    if (dst.size() != size)
        dst.resize(size);

    for (int yDst = 0; yDst < dstHeight; ++yDst)
    for (int xDst = 0; xDst < dstWidth; ++xDst)
    {
        const int xSrc = xDst * 2;
        const int ySrc = yDst * 2;

        // Calculate the average pixel value
        const glm::ivec2 p00(xSrc,     ySrc);
        const glm::ivec2 p10(xSrc + 1, ySrc);
        const glm::ivec2 p01(xSrc,     ySrc + 1);
        const glm::ivec2 p11(xSrc + 1, ySrc + 1);
        std::array<T, C> pix =
            average<T, C>(p00, p10, p01, p11,
                          srcWidth,
                          src);

        // Set the average pixel vaue
        for (int i = 0; i < C; ++i)
            dst[yDst * dstWidth * C + C * xDst + i] = pix[i];
    }
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class MipmapGenerator
{
public:
    static const int MIN_MIPMAP_SIZE = 16;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    template<typename T, int C>
    bool generate(Texture2D<T, C>& tex)
    {
        std::vector<Texture2D<T, C>> mipmaps;

        const int w = tex.d->width;
        const int h = tex.d->height;
        int size = w;

        // Dimensions must match
        if (w != h)
            return false;

        // Size must be power of 2
        if ((size & (size - 1)) != 0)
            return false;

        // Min size check
        if (size < MIN_MIPMAP_SIZE)
            return false;

        const std::vector<T>* copy = &tex.d->pixels;
        while(size > MIN_MIPMAP_SIZE)
        {
            int oldSize = size;
            size = size >> 1;

            Texture2D<T, C> mipmap(size, size);
            scaleMinify<T, C>(oldSize, oldSize, *copy, mipmap.d->pixels);

            mipmaps.push_back(mipmap);

            copy = &mipmaps[mipmaps.size() - 1].d->pixels;
        }

        tex.d->mipmaps = mipmaps;

        return true;
    }
};

} // namespace rasperi
} // namespace kuu
