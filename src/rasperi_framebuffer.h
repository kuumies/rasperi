/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::Framebuffer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <QtGui/QImage>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
template<typename T>
class Framebuffer
{
public:
    using Data    = std::vector<T>;
    using DataPtr = std::shared_ptr<Data>;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Framebuffer(int width, int height, int channels)
        : width(width)
        , height(height)
        , channels(channels)
    {
        const std::size_t size = std::size_t(width * height * channels);
        data = std::make_shared<Data>(size);
        clear();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void clear()
    {
        memset(data.get()->data(), 0, size());
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void set(T value)
    {
        Data& d = *data.get();
        for (int y = 0; y < height;   ++y)
        for (int x = 0; x < width;    ++x)
        for (int c = 0; c < channels; ++c)
            d[y * width + x * channels + c] = value;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void set(int x, int y, int c, T value)
    {
        Data& d = *data.get();
        d[y * width * channels + x * channels + c] = value;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    T get(int x, int y, int c)
    {
        Data& d = *data.get();
        return d[y * width * channels + x * channels + c];
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::size_t size() const
    { return std::size_t(width * height * channels); }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    QImage toQImage() const
    {
        if (channels != 4)
            return QImage();

        QImage out = QImage(data.get()->data(),
                            width, height,
                            QImage::Format_ARGB32).copy();
        return out.rgbSwapped();
    }

    int width;
    int height;
    int channels;
    DataPtr data;
};

using DepthFramebuffer = Framebuffer<double>;
using ColorFramebuffer = Framebuffer<unsigned char>;

} // namespace rasperi
} // namespace kuu
