/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLSkyBox class
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace kuu
{
namespace rasperi
{ 

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLSkyBoxShader
{
public:
    OpenGLSkyBoxShader();

    void use();

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;


private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
