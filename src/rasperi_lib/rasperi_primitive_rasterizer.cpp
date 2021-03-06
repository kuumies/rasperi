/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::PrimitiveRasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_primitive_rasterizer.h"
#include <array>
#include <iostream>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PrimitiveRasterizer::PrimitiveRasterizer(Framebuffer& framebuffer)
    : framebuffer(framebuffer)
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PrimitiveRasterizer::~PrimitiveRasterizer()
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec3 PrimitiveRasterizer::project(const glm::dmat4& m, const glm::dvec3& p)
{
    const glm::dvec4 v = m * glm::dvec4(p, 1.0);
    if (v.w == 0.0)
    {
        std::cerr << "proj err" << std::endl;
        return glm::dvec3(0.0);
    }
    return glm::dvec3(v.x / v.w, v.y / v.w, v.z / v.w);
}


/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec3 PrimitiveRasterizer::transform(const glm::dmat4& m,
                                          const glm::dvec3& p)
{
    return m * glm::dvec4(p, 1.0);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec2 PrimitiveRasterizer::viewportTransform(const glm::dvec3& p)
{
    const glm::dvec2 vp(framebuffer.colorTex.width(), framebuffer.colorTex.height());
    const glm::dvec2 halfViewport = vp * 0.5;

    glm::dvec2 out;
    out.x =        std::floor((p.x + 1.0) * halfViewport.x);
    out.y = vp.y - std::floor((p.y + 1.0) * halfViewport.y);

    glm::dvec2 vpMin = glm::dvec2(0, 0);
    glm::dvec2 vpMax = glm::dvec2(vp.x - 1, vp.y- 1);
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

    std::array<uchar, 4> pix = { r, g,b, a };
    framebuffer.colorTex.setPixel(x, y, pix);
}

} // namespace rasperi
} // namespace kuu
