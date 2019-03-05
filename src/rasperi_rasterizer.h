/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Rasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include "rasperi_framebuffer.h"

namespace kuu
{
namespace rasperi
{

struct Material;
struct Mesh;

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

    enum class IlluminationMode
    {
        Phong,
        Pbr,
    };

    Rasterizer(int width, int height);
    void clear();
    void setModelMatrix(const glm::dmat4& view);
    void setViewMatrix(const glm::dmat4& view);
    void setProjectionMatrix(const glm::dmat4& projection);
    void setMaterial(const Material& material);
    void setNormalMode(NormalMode normalMode);
    void drawFilledTriangleMesh(Mesh* mesh);
    void drawEdgeLineTriangleMesh(Mesh* mesh);
    void drawLineMesh(Mesh* mesh);

    ColorFramebuffer colorFramebuffer() const;
    DepthFramebuffer depthFramebuffer() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
