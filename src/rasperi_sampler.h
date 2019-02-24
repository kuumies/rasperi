/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::Sampler class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <QtGui/QImage>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Sampler
{
public:
    enum class Filter
    {
        Nearest,
        Linear
    };

    Sampler(const QImage& map,
            Filter filter = Filter::Nearest,
            bool linearizeGamma = false);

    void setFilter(Filter filter);
    Filter filter() const;

    void setLinearizeGamma(bool linearize);
    bool linearizeGamma() const;

    glm::dvec4 sampleRgba(const glm::dvec2& texCoord) const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
