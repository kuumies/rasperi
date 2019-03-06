/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLReferenceRasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include <glad/glad.h>

namespace kuu
{
namespace rasperi
{

struct Material;
struct Mesh;
class OpenGLFramebuffer;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLReferenceRasterizer
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

    OpenGLReferenceRasterizer(int width, int height);
    void clear();
    void setModelMatrix(const glm::dmat4& view);
    void setViewMatrix(const glm::dmat4& view);
    void setProjectionMatrix(const glm::dmat4& projection);
    void setMaterial(const Material& material);
    void setNormalMode(NormalMode normalMode);
    void drawFilledTriangleMesh(Mesh* mesh);
    void drawEdgeLineTriangleMesh(Mesh* mesh);
    void drawLineMesh(Mesh* mesh);
    void run(GLuint fbo);

    OpenGLFramebuffer& framebuffer() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
