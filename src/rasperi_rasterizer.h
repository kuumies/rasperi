/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::wakusei::rasterizer::Rasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include "rasperi_rasterizer_types.h"

namespace kuu
{
namespace wakusei
{
namespace rasterizer
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
    void drawTriangleMesh(Mesh* mesh);
    void drawLineMesh(Mesh* mesh);

    ColorFramebuffer colorFramebuffer() const;
    DepthFramebuffer depthFramebuffer() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasterizer
} // namespace wakusei
} // namespace kuu
