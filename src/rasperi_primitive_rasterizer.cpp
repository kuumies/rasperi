/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::PrimitiveRasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_primitive_rasterizer.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PrimitiveRasterizer::PrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                                         DepthFramebuffer& depthbuffer)
    : colorbuffer(colorbuffer)
    , depthbuffer(depthbuffer)
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PrimitiveRasterizer::~PrimitiveRasterizer()
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec3 PrimitiveRasterizer::project(const glm::dmat4& m,
                   const glm::dvec3& p)
{
    const glm::dvec4 v = m * glm::dvec4(p, 1.0);
    if (v.w == 0.0)
        return glm::dvec3(0.0);
    return glm::dvec3(v.x / v.w, v.y / v.w, v.z / v.w);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::ivec2 PrimitiveRasterizer::viewportTransform(const glm::dvec3& p)
{
    const glm::ivec2 vp(colorbuffer.width, colorbuffer.height);
    const glm::dvec2 halfViewport = glm::dvec2(vp) * 0.5;

    glm::ivec2 out;
    out.x =        int(std::floor((p.x + 1.0) * halfViewport.x));
    out.y = vp.y - int(std::floor((p.y + 1.0) * halfViewport.y));

    glm::ivec2 vpMin = glm::ivec2(0, 0);
    glm::ivec2 vpMax = glm::ivec2(vp.x - 1, vp.y- 1);
    return glm::clamp(out, vpMin, vpMax);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void PrimitiveRasterizer::setRgba(int x, int y, glm::dvec4 c)
{
    c.r = glm::clamp(c.r, 0.0, 1.0);
    c.g = glm::clamp(c.g, 0.0, 1.0);
    c.b = glm::clamp(c.b, 0.0, 1.0);
    c.a = glm::clamp(c.a, 0.0, 1.0);

    using uchar = unsigned char;
    unsigned char r = uchar(std::floor(c.r * 255.0));
    unsigned char g = uchar(std::floor(c.g * 255.0));
    unsigned char b = uchar(std::floor(c.b * 255.0));
    unsigned char a = uchar(std::floor(c.a * 255.0));

    size_t loc = size_t(y * colorbuffer.width * colorbuffer.channels +
                        x * colorbuffer.channels);

    ColorFramebuffer::Data& d = *colorbuffer.data.get();
    d[loc + 0] = r;
    d[loc + 1] = g;
    d[loc + 2] = b;
    d[loc + 3] = a;
}

} // namespace rasperi
} // namespace kuu
