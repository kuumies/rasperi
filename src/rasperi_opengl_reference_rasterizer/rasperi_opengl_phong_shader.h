/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLPhongShader class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include <glad/glad.h>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLPhongShader
{
public:
    OpenGLPhongShader();

    void use();

    glm::vec3 cameraPosition;
    glm::vec3 lightDirection;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float specularPower;
    bool useAmbientSampler;
    bool useDiffuseSampler;
    bool useSpecularSampler;
    bool useSpecularPowerSampler;
    bool useNormalSampler;
    bool rgbSpecularSampler;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
