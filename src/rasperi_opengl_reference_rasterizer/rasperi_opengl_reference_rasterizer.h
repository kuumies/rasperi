/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLReferenceRasterizer class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glad/glad.h>
#include "rasperi_lib/rasperi_model.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLReferenceRasterizer
{
public:
    struct Scene
    {
        glm::ivec4 viewport;
        glm::vec3 lightDirection;
        glm::vec3 cameraPosition;
        glm::dmat4 view;
        glm::dmat4 projection;
        std::vector<Model> models;
        QImage background;
        Texture2D<double, 4> skyTexture;
    };

    OpenGLReferenceRasterizer();

    void run(GLuint fbo, const Scene& scene);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
