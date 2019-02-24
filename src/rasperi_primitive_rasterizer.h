/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::PrimitiveRasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "rasperi_framebuffer.h"
#include "rasperi_mesh.h"
#include "rasperi_rasterizer.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class PrimitiveRasterizer
{
public:
    PrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                        DepthFramebuffer& depthbuffer);
    virtual ~PrimitiveRasterizer();

    glm::dvec3 project(const glm::dmat4& m, const glm::dvec3& p);
    glm::ivec2 viewportTransform(const glm::dvec3& p);
    void setRgba(int x, int y, glm::dvec4 c);

protected:
    ColorFramebuffer& colorbuffer;
    DepthFramebuffer& depthbuffer;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class LinePrimitiveRasterizer : public PrimitiveRasterizer
{
public:
    LinePrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                            DepthFramebuffer& depthbuffer);

    void rasterize(const Mesh& m, const glm::dmat4& matrix);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class TrianglePrimitiveRasterizer : public PrimitiveRasterizer
{
public:
    TrianglePrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                                DepthFramebuffer& depthbuffer,
                                Rasterizer::NormalMode normalMode);

    void rasterize(const Mesh& triangleMesh,
                   const glm::dmat4& matrix,
                   const glm::dmat3& normalMatrix,
                   const glm::dvec3& lightDir);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
