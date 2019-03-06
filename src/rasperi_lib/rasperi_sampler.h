/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Sampler class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class QImage;

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

    Sampler();
    Sampler(const QImage& map,
            Filter filter = Filter::Linear,
            bool linearizeGamma = false);

    bool isValid() const;

    void setMap(const QImage& map);
    QImage map() const;

    void setFilter(Filter filter);
    Filter filter() const;

    void setLinearizeGamma(bool linearize);
    bool linearizeGamma() const;

    glm::dvec4 sampleRgba(const glm::dvec2& texCoord) const;
    double sampleGrayscale(const glm::dvec2& texCoord) const;

    void writeRgba(const glm::dvec2& texCood,
                   const glm::dvec4& rgba);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
