/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::Sampler class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_sampler.h"
#include <array>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/geometric.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Sampler::Impl
{
    Impl(const QImage& map,
         Filter filter,
         bool linearizeGamma)
        : map(map)
        , filter(filter)
        , linearizeGamma(linearizeGamma)
    {}

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::dvec2 wrapTexCoord(glm::dvec2 texCoord) const
    {
        texCoord.x = fmod(texCoord.x, 1.0);
        if (texCoord.x < 0.0)
           texCoord.x += 1.0;
        texCoord.y = fmod(texCoord.y, 1.0);
        if (texCoord.y < 0.0)
           texCoord.y += 1.0;
        return texCoord;
    }

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::dvec2 clampTexCoord(glm::dvec2 texCoord) const
    {
        texCoord.x = glm::clamp(texCoord.x, 0.0, 1.0);
        texCoord.y = glm::clamp(texCoord.y, 0.0, 1.0);
        return texCoord;
    }

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::ivec2 mapCoord(const glm::dvec2& texCoord) const
    {
        int px = int(std::floor(texCoord.x * double(map.width()  - 1)));
        int py = int(std::floor(texCoord.y * double(map.height() - 1)));
        return glm::ivec2(px, py);
    }

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::dvec4 sampleRgbaNearest(glm::dvec2 texCoord) const
    {
        //texCoord = wrapTexCoord(texCoord);
        //texCoord = clampTexCoord(texCoord);
        //texCoord.x = 1.0 - texCoord.x;
        texCoord.y = 1.0 - texCoord.y;

        int px = int(std::floor(texCoord.x * double(map.width()  - 1)));
        int py = int(std::floor(texCoord.y * double(map.height() - 1)));
        return sampleRgba(px, py);
    }

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::dvec4 sampleRgbaLinear(glm::dvec2 texCoord) const
    {
        texCoord.y = 1.0 - texCoord.y;
        texCoord = wrapTexCoord(texCoord);

        int px = int(std::floor(texCoord.x * double(map.width()  - 1)));
        int py = int(std::floor(texCoord.y * double(map.height() - 1)));

        std::array<glm::dvec4, 4> pixels;
        pixels[0] = sampleRgba(px,     py    );
        pixels[1] = sampleRgba(px + 1, py    );
        pixels[2] = sampleRgba(px,     py + 1);
        pixels[3] = sampleRgba(px + 1, py + 1);

        return bilinear<glm::dvec4>(texCoord.x, texCoord.y,
                                    pixels[0],
                                    pixels[1],
                                    pixels[2],
                                    pixels[3]);
    }

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::dvec4 sampleRgba(int x, int y) const
    {      
        //x = map.width() - x - 1;

        if (map.isNull() || map.width() <= 0 || map.height() <= 0)
        {
            std::cerr << __FUNCTION__ << ": "
                      << "map error"
                      << std::endl;
            return glm::dvec4(0.0);
        }

        if (map.isNull() || x < 0 || x >= map.width() || y < 0 || y >= map.height())
        {
            std::cerr << __FUNCTION__ << ": "
                      << x << ", " << y << ", "
                      << map.width() << ", " << map.height()
                      << std::endl << std::flush;
            return glm::dvec4(0.0);
        }

        const QRgb* line = reinterpret_cast<const QRgb*>(map.scanLine(y));
        const QRgb pixel = line[x];

        glm::dvec4 out(
            double(qRed(pixel))   / 255.0,
            double(qGreen(pixel)) / 255.0,
            double(qBlue(pixel))  / 255.0,
            double(qAlpha(pixel)) / 255.0);
        out = glm::clamp(out, glm::dvec4(0.0), glm::dvec4(1.0));

        if (linearizeGamma)
        {
            double gamma = 2.2;
            out = glm::pow(out, glm::dvec4(gamma));
        }

        return out;
    }

    /* ---------------------------------------------------------------- *
       https://www.scratchapixel.com/lessons/mathematics-physics-for-
       computer-graphics/interpolation/bilinear-filtering
     * ---------------------------------------------------------------- */
    template<typename T>
    T bilinear(
       const double tx,
       const double ty,
       const T& c00,
       const T& c10,
       const T& c01,
       const T& c11) const
    {
        T a = c00 * (1.0 - tx) + c10 * tx;
        T b = c01 * (1.0 - tx) + c11 * tx;
        return a * (1.0 - ty) + b * ty;
    }

    QImage map;
    Filter filter;
    bool linearizeGamma;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Sampler::Sampler()
    : Sampler(QImage())
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Sampler::Sampler(const QImage& map,
        Filter filter,
        bool linearizeGamma)
    : impl(std::make_shared<Impl>(map, filter, linearizeGamma))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool Sampler::isValid() const
{ return !impl->map.isNull(); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Sampler::setMap(const QImage& map)
{ impl->map = map; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage Sampler::map() const
{ return impl->map; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Sampler::setFilter(Sampler::Filter filter)
{ impl->filter = filter; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Sampler::Filter Sampler::filter() const
{ return impl->filter; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Sampler::setLinearizeGamma(bool linearize)
{ impl->linearizeGamma = linearize; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool Sampler::linearizeGamma() const
{ return impl->linearizeGamma; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec4 Sampler::sampleRgba(const glm::dvec2& texCoord) const
{
    switch(impl->filter)
    {
        case Filter::Nearest: return impl->sampleRgbaNearest(texCoord);
        case Filter::Linear:  return impl->sampleRgbaLinear(texCoord);
    }
    return glm::dvec4(0.0);
}

} // namespace rasperi
} // namespace kuu
