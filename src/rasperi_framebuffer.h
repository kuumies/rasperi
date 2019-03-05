/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Framebuffer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <array>
#include "rasperi_texture_2d.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Framebuffer
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Framebuffer(int width, int height)
        : colorTex(width, height)
        , depthTex(width, height)
    {
        clear();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void clear()
    {
        std::array<uchar, 4> colorPix = { 0, 0, 0, 0 };
        colorTex.clear(colorPix);

        std::array<double, 1> depthPix = { std::numeric_limits<double>::max() };
        depthTex.clear(depthPix);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Texture2D<uchar,  4> colorTex;
    Texture2D<double, 1> depthTex;
};

} // namespace rasperi
} // namespace kuu
