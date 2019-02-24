/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Rasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include "rasperi_framebuffer.h"
#include "rasperi_mesh.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
   A rasterizer for rendering widgets with three-dimensional
   graphic e.g light direction, camera frustum, etc.
 * ---------------------------------------------------------------- */
class Rasterizer
{
public:
    enum class NormalMode
    {
        Smooth,
        Coarse,
    };

    Rasterizer(int width, int height);
    void clear();
    void setViewMatrix(const glm::dmat4& view);
    void setProjectionMatrix(const glm::dmat4& projection);
    void setNormalMode(NormalMode normalMode);
    void drawTriangleMesh(Mesh* mesh);
    void drawLineMesh(Mesh* mesh);

    ColorFramebuffer colorFramebuffer() const;
    DepthFramebuffer depthFramebuffer() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
