/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::PrimitiveRasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "rasperi_framebuffer.h"
#include "rasperi_rasterizer.h"

namespace kuu
{
namespace rasperi
{

struct Mesh;
struct Material;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class PrimitiveRasterizer
{
public:
    PrimitiveRasterizer(Framebuffer& framebuffer);
    virtual ~PrimitiveRasterizer();

    glm::dvec3 project(const glm::dmat4& m, const glm::dvec3& p);
    glm::dvec3 transform(const glm::dmat4&m, const glm::dvec3& p);
    glm::dvec2 viewportTransform(const glm::dvec3& p);
    void setRgba(int x, int y, glm::dvec4 c);

protected:
    Framebuffer& framebuffer;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class LinePrimitiveRasterizer : public PrimitiveRasterizer
{
public:
    LinePrimitiveRasterizer(Framebuffer& framebuffer);

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
    TrianglePrimitiveRasterizer(Framebuffer& framebuffer,
                                Rasterizer::NormalMode normalMode);

    void rasterize(const Mesh& triangleMesh,
                   const glm::dmat4& cameraMatrix,
                   const glm::dmat4& modelMatrix,
                   const glm::dmat3& normalMatrix,
                   const glm::dvec3& lightDir,
                   const glm::dvec3& cameraPos,
                   const Material& material);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
