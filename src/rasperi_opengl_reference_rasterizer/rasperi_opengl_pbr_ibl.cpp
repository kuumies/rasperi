/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLPbrImageLighting
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_pbr_ibl.h"
#include <array>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QtGui/QImage>
#include "rasperi_lib/rasperi_camera.h"
#include "rasperi_lib/rasperi_cube_camera.h"
#include "rasperi_opengl_ndc_mesh.h"
#include "rasperi_opengl_shader_loader.h"
#include "rasperi_opengl_texture_to_qimage.h"

namespace kuu
{
namespace rasperi
{
namespace
{

/* ---------------------------------------------------------------- *
   Renders a PBR irradiance texture cube.
 * ---------------------------------------------------------------- */
GLuint renderPbrIrradiance(std::shared_ptr<NdcCubeMesh> ndcBox,
                           const glm::ivec2& size,
                           GLuint environmentTex)
{
    // ---------------------------------------------------------------
    // Color texture cube framebuffer attachment

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_WRAP_R,
                    GL_CLAMP_TO_EDGE);

    for (GLenum face = 0; face < 6; ++face)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
                     GL_RGB16F, size.x, size.y, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }

    // ---------------------------------------------------------------
    // Depth renberbuffer framebuffer attachment

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          size.x, size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // ---------------------------------------------------------------
    // Framebuffer

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------------------------------------------------------------
    // Shader program

    GLuint pgm = 
        opengl_shader_loader::load(
            "shaders/rasperi_opengl_pbr_ibl_irradiance.vsh", 
            "shaders/rasperi_opengl_pbr_ibl_irradiance.fsh");

    GLint matricesProjectionLoc = glGetUniformLocation(pgm, "matrices.projection");
    GLint matricesViewLoc       = glGetUniformLocation(pgm, "matrices.view");
    GLint textureCubeLoc        = glGetUniformLocation(pgm, "skyboxMap");

    // ---------------------------------------------------------------
    // Render texture cube faces

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentTex);

    CubeCamera cubeCamera(1.0);

    for (GLenum face = 0; face < 6; ++face)
    {
        glm::mat4 projection = cubeCamera.projectionMatrix;
        glm::mat4 view       = cubeCamera.viewMatrices[face];

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                               tex,
                               0);
        glViewport(0, 0, size.x, size.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(pgm);
        glUniform1i(textureCubeLoc, 0);
        glUniformMatrix4fv(matricesViewLoc,       1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(matricesProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        ndcBox->draw();
    }

    opengl_texture_to_qimage::textureCube(tex).save("/temp/00_irradiance.bmp");

    glDeleteProgram(pgm);

    return tex;
}

/* ---------------------------------------------------------------- *
   Renders a PBR prefilter texture cube.
 * ---------------------------------------------------------------- */
GLuint renderPbrPrefilter(std::shared_ptr<NdcCubeMesh> ndcBox,
                          const glm::ivec2& size,
                          GLuint environmentTex,
                          int& prefilterMipmapCount)
{
    // ---------------------------------------------------------------
    // Color texture cube framebuffer attachment

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_WRAP_R,
                    GL_CLAMP_TO_EDGE);

    for (GLenum face = 0; face < 6; ++face)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
                     GL_RGB16F, size.x, size.y, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // ---------------------------------------------------------------
    // Depth renberbuffer framebuffer attachment

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);

    // ---------------------------------------------------------------
    // Framebuffer

    GLuint fbo;
    glGenFramebuffers(1, &fbo);


    // ---------------------------------------------------------------
    // Shader program.

    GLuint pgm = 
        opengl_shader_loader::load(
            "shaders/rasperi_opengl_pbr_ibl_prefilter.vsh", 
            "shaders/rasperi_opengl_pbr_ibl_prefilter.fsh");

    GLint matricesProjectionLoc = glGetUniformLocation(pgm, "matrices.projection");
    GLint matricesViewLoc       = glGetUniformLocation(pgm, "matrices.view");
    GLint textureCubeLoc        = glGetUniformLocation(pgm, "skyboxMap");
    GLint roughnessLoc          = glGetUniformLocation(pgm, "roughness");

    // ---------------------------------------------------------------
    // Render

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentTex);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    CubeCamera cubeCamera(1.0);

    const int mipmapLevels = 5;
    prefilterMipmapCount = mipmapLevels;
    for (int mipmap = 0; mipmap < mipmapLevels; ++mipmap)
    {
        GLsizei mipmapWidth  = GLsizei(size.x * std::pow(0.5, mipmap));
        GLsizei mipmapHeight = GLsizei(size.y * std::pow(0.5, mipmap));
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER,
                              GL_DEPTH_COMPONENT24,
                              mipmapWidth,
                              mipmapHeight);
        glViewport(0, 0, mipmapWidth, mipmapHeight);

        for (GLenum face = 0; face < 6; ++face)
        {
            glm::mat4 projection = cubeCamera.projectionMatrix;
            glm::mat4 view       = cubeCamera.viewMatrices[face];

            float roughness = mipmap / float(mipmapLevels - 1);

            glFramebufferTexture2D(GL_FRAMEBUFFER, 
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   tex, mipmap);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(pgm);

            glUniform1f(roughnessLoc, roughness);
            glUniform1i(textureCubeLoc, 0);
            glUniformMatrix4fv(matricesViewLoc,       1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(matricesProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

            ndcBox->draw();
        }
    }

    glDeleteProgram(pgm);

    for (int i = 0; i < 4; ++i)
        opengl_texture_to_qimage::textureCube(tex, i).save(QString("/temp/00_prefilter_%1.bmp").arg(i));

    glCullFace(GL_BACK);
    return tex;
}

/* ---------------------------------------------------------------- *
   Renders a PBR BRDF integration 2D texture.
 * ---------------------------------------------------------------- */
GLuint renderPbrBrdfIntegration(std::shared_ptr<NdcQuadMesh> ndcQuad,
                                const glm::ivec2& size)
{
    // ---------------------------------------------------------------
    // Color texture framebuffer attachment

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size.x, size.y, 0,
                 GL_RG, GL_FLOAT, nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // ---------------------------------------------------------------
    // Depth renderbuffer framebuffer attachment

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, 
                          GL_DEPTH_COMPONENT16,
                          size.x, size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // ---------------------------------------------------------------
    // Framebuffer

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           tex,
                           0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------------------------------------------------------------
    // Shader program

    GLuint pgm = 
        opengl_shader_loader::load(
            "shaders/rasperi_opengl_pbr_ibl_brdf_integration.vsh", 
            "shaders/rasperi_opengl_pbr_ibl_brdf_integration.fsh");

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glViewport(0, 0, size.x, size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(pgm);

    ndcQuad->draw();

    glDeleteProgram(pgm);

    return tex;
}

} // anonymous namespace

/* ---------------------------------------------------------------- *
   PbrImageLightingTextures
 * ---------------------------------------------------------------- */

PbrImageLightingTextures::~PbrImageLightingTextures()
{
    std::array<GLuint, 3> textures =
    { irradianceTex, prefilterTex, brdfIntegrationTex };
    glDeleteTextures(3, textures.data());
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void PbrImageLightingTextures::bind()
{
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceTex);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterTex);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, brdfIntegrationTex);
}

/* ---------------------------------------------------------------- *
   Functions
 * ---------------------------------------------------------------- */

std::shared_ptr<PbrImageLightingTextures> renderPbrImageLighting(
    std::shared_ptr<NdcQuadMesh> ndcQuad,
    std::shared_ptr<NdcCubeMesh> ndcBox,
    const glm::ivec2& irradianceMapSize,
    const glm::ivec2& prefilterMapSize,
    const glm::ivec2& brdfIntegrationMapSize,
    GLuint environmentTex)
{
    std::shared_ptr<PbrImageLightingTextures> textures =
        std::make_shared<PbrImageLightingTextures>();

    textures->irradianceTex = renderPbrIrradiance(
        ndcBox,
        irradianceMapSize,
        environmentTex);

    textures->prefilterTex = renderPbrPrefilter(
        ndcBox,
        prefilterMapSize,
        environmentTex,
        textures->prefilterMipmapCount);

    textures->brdfIntegrationTex = renderPbrBrdfIntegration(
        ndcQuad,
        brdfIntegrationMapSize);

    return textures;
}

} // namespace rasperi
} // namespace kuu
