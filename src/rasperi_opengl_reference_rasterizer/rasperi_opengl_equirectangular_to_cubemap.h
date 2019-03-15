/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::EquirectangularToCubemap class
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glad/glad.h>
#include <glm/vec2.hpp>
#include "rasperi_lib/rasperi_texture_2d.h"

namespace kuu
{
namespace rasperi
{ 

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLEquirectangularToCubemap
{
public:
    OpenGLEquirectangularToCubemap(int size);
    void run(const Texture2D<double, 4>& tex);
    void run(const GLuint tex);

    GLuint cubemap;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
