/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::Rasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace kuu
{
namespace rasperi
{
namespace rasterizer
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Vertex
{
    glm::dvec3 position;
    glm::dvec2 texCoord;
    glm::dvec3 normal;
    glm::dvec4 color;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Triangle
{
    Vertex p1;
    Vertex p2;
    Vertex p3;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Mesh
{
    std::vector<unsigned> indices;
    std::vector<Vertex> vertices;
};

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

    int width;
    int height;
    int channels;
    DataPtr data;
};

using DepthFramebuffer = Framebuffer<double>;
using ColorFramebuffer = Framebuffer<unsigned char>;


} // namespace rasterizer
} // namespace rasperi
} // namespace kuu
