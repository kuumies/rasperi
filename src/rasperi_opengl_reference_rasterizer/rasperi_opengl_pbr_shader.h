/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLPbrShader class.
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
class OpenGLPbrShader
{
public:
    OpenGLPbrShader();

    void use();

    glm::vec3 cameraPosition;
    glm::vec3 lightDirection;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 albedo;
    float roughness;
    float metalness;
    float ao;
    bool useAlbedoSampler;
    bool useRoughnessSampler;
    bool useMetalnessSampler;
    bool useAoSampler;
    bool useNormalSampler;
    int prefilterSamplerMipmapCount;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
