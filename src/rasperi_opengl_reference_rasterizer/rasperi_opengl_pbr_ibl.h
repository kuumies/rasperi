/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLPbrImageLighting
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glad/glad.h>
#include <glm/vec2.hpp>

namespace kuu
{
namespace rasperi
{ 

struct NdcQuadMesh;
struct NdcCubeMesh;

/* ---------------------------------------------------------------- *
   Output of the PBR image lighting calculation. Contains image
   lighting textures:

    1) Irradiance RGB texture cube
    2) Prefilter RGB texture cube
    3) BRDF integration RG 2D texture

 * ---------------------------------------------------------------- */
struct PbrImageLightingTextures
{
    ~PbrImageLightingTextures();

    void bind();

    GLuint irradianceTex;
    GLuint prefilterTex;
    GLuint brdfIntegrationTex;
    int prefilterMipmapCount;
};

/* ---------------------------------------------------------------- *
   Calculates the PBR image lighting into textures.
 * ---------------------------------------------------------------- */
std::shared_ptr<PbrImageLightingTextures> renderPbrImageLighting(
    std::shared_ptr<NdcQuadMesh> ndcQuad,
    std::shared_ptr<NdcCubeMesh> ndcBox,
    const glm::ivec2& irradianceMapSize,
    const glm::ivec2& prefilterMapSize,
    const glm::ivec2& brdfIntegrationMapSize,
    GLuint environmentTex);

} // namespace rasperi
} // namespace kuu
