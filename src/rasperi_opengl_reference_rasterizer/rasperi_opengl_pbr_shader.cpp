/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLPbrShader class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_pbr_shader.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "rasperi_opengl_shader_loader.h"
#include <QDebug>

namespace kuu
{
namespace rasperi
{

using namespace opengl_shader_loader;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLPbrShader::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(OpenGLPbrShader* self)
        : self(self)
    {
        pgm = load("shaders/rasperi_opengl_pbr_shader.vsh",
                   "shaders/rasperi_opengl_pbr_shader.fsh");

        uniformProjectionMatrix            = glGetUniformLocation(pgm, "matrices.projection");
        uniformViewMatrix                  = glGetUniformLocation(pgm, "matrices.view");
        uniformModelMatrix                 = glGetUniformLocation(pgm, "matrices.model");
        uniformNormalMatrix                = glGetUniformLocation(pgm, "matrices.normal");
        uniformCameraPosition              = glGetUniformLocation(pgm, "cameraPosition");
        uniformSunDirection                = glGetUniformLocation(pgm, "sunDirection");
        uniformAlbedo                      = glGetUniformLocation(pgm, "albedoValue");
        uniformRoughness                   = glGetUniformLocation(pgm, "roughnessValue");
        uniformMetalness                   = glGetUniformLocation(pgm, "metalnessValue");
        uniformAo                          = glGetUniformLocation(pgm, "aoValue");
        uniformAlbedoSampler               = glGetUniformLocation(pgm, "albedoSampler");
        uniformRoughnessSampler            = glGetUniformLocation(pgm, "roughnessSampler");
        uniformMetalnessSampler            = glGetUniformLocation(pgm, "metalnessSampler");
        uniformAoSampler                   = glGetUniformLocation(pgm, "aoSampler");
        uniformNormalSampler               = glGetUniformLocation(pgm, "normalSampler");
        uniformUseAlbedoSampler            = glGetUniformLocation(pgm, "useAlbedoSampler");
        uniformUseRoughnessSampler         = glGetUniformLocation(pgm, "useRoughnessSampler");
        uniformUseMetalnessSampler         = glGetUniformLocation(pgm, "useMetalnessSampler");
        uniformUseAoSampler                = glGetUniformLocation(pgm, "useAoSampler");
        uniformUseNormalSampler            = glGetUniformLocation(pgm, "useNormalSampler");
        uniformIrradianceSampler           = glGetUniformLocation(pgm, "irradianceSampler");
        uniformPrefilterSampler            = glGetUniformLocation(pgm, "prefilterSampler");
        uniformBrdfIntegrationSampler      = glGetUniformLocation(pgm, "brdfIntegrationSampler");
        uniformPrefilterSamplerMipmapCount = glGetUniformLocation(pgm, "prefilterSamplerMipmapCount");

        qDebug() << uniformIrradianceSampler
                 << uniformPrefilterSampler;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    ~Impl()
    {
        glDeleteProgram(pgm);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void use()
    {
        const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(self->modelMatrix));

        glUseProgram(pgm);
        glUniformMatrix4fv(uniformModelMatrix,      1, GL_FALSE, glm::value_ptr(self->modelMatrix));
        glUniformMatrix4fv(uniformViewMatrix,       1, GL_FALSE, glm::value_ptr(self->viewMatrix));
        glUniformMatrix4fv(uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(self->projectionMatrix));
        glUniformMatrix3fv(uniformNormalMatrix,     1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniform3fv(      uniformCameraPosition,   1,           glm::value_ptr(self->cameraPosition));
        glUniform3fv(      uniformSunDirection,     1,           glm::value_ptr(self->lightDirection));
        glUniform3fv(      uniformAlbedo,           1,           glm::value_ptr(self->albedo));
        glUniform1f(       uniformRoughness,                     self->roughness);
        glUniform1f(       uniformMetalness,                     self->metalness);
        glUniform1f(       uniformAo,                            self->ao);
        glUniform1i(       uniformAlbedoSampler,                 0);
        glUniform1i(       uniformRoughnessSampler,              1);
        glUniform1i(       uniformMetalnessSampler,              2);
        glUniform1i(       uniformAoSampler,                     3);
        glUniform1i(       uniformNormalSampler,                 4);
        glUniform1i(       uniformUseAlbedoSampler,              self->useAlbedoSampler);
        glUniform1i(       uniformUseRoughnessSampler,           self->useRoughnessSampler);
        glUniform1i(       uniformUseMetalnessSampler,           self->useMetalnessSampler);
        glUniform1i(       uniformUseAoSampler,                  self->useAoSampler);
        glUniform1i(       uniformUseNormalSampler,              self->useNormalSampler);
        glUniform1i(       uniformIrradianceSampler,             5);
        glUniform1i(       uniformPrefilterSampler,              6);
        glUniform1i(       uniformBrdfIntegrationSampler,        7);
        glUniform1i(       uniformPrefilterSamplerMipmapCount,   self->prefilterSamplerMipmapCount);

    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    OpenGLPbrShader* self;
    GLuint pgm;
    GLint uniformProjectionMatrix;
    GLint uniformViewMatrix;
    GLint uniformModelMatrix;
    GLint uniformNormalMatrix;
    GLint uniformCameraPosition;
    GLint uniformSunDirection;
    GLint uniformAlbedo;
    GLint uniformRoughness;
    GLint uniformMetalness;
    GLint uniformAo;
    GLint uniformAlbedoSampler;
    GLint uniformRoughnessSampler;
    GLint uniformMetalnessSampler;
    GLint uniformAoSampler;
    GLint uniformNormalSampler;
    GLint uniformUseAlbedoSampler;
    GLint uniformUseRoughnessSampler;
    GLint uniformUseMetalnessSampler;
    GLint uniformUseAoSampler;
    GLint uniformUseNormalSampler;
    GLint uniformIrradianceSampler;
    GLint uniformPrefilterSampler;
    GLint uniformBrdfIntegrationSampler;
    GLint uniformPrefilterSamplerMipmapCount;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLPbrShader::OpenGLPbrShader()
    : impl(std::make_shared<Impl>(this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLPbrShader::use()
{ impl->use(); }

} // namespace rasperi
} // namespace kuu
