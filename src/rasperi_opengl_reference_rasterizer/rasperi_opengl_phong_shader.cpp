/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLPhongShader class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_phong_shader.h"
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
struct OpenGLPhongShader::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(OpenGLPhongShader* self)
        : self(self)
    {
        pgm = load("shaders/rasperi_opengl_phong_shader.vsh",
                   "shaders/rasperi_opengl_phong_shader.fsh");

        uniformProjectionMatrix        = glGetUniformLocation(pgm, "matrices.projection");
        uniformViewMatrix              = glGetUniformLocation(pgm, "matrices.view");
        uniformModelMatrix             = glGetUniformLocation(pgm, "matrices.model");
        uniformNormalMatrix            = glGetUniformLocation(pgm, "matrices.normal");
        uniformCameraPosition          = glGetUniformLocation(pgm, "cameraPosition");
        uniformSunDirection            = glGetUniformLocation(pgm, "sunDirection");
        uniformAmbient                 = glGetUniformLocation(pgm, "ambient");
        uniformDiffuse                 = glGetUniformLocation(pgm, "diffuse");
        uniformSpecular                = glGetUniformLocation(pgm, "specular");
        uniformSpecularPower           = glGetUniformLocation(pgm, "specularPower");
        uniformAmbientSampler          = glGetUniformLocation(pgm, "ambientSampler");
        uniformDiffuseSampler          = glGetUniformLocation(pgm, "diffuseSampler");
        uniformSpecularSampler         = glGetUniformLocation(pgm, "specularSampler");
        uniformSpecularPowerSampler    = glGetUniformLocation(pgm, "speculerPowerSampler");
        uniformNormalSampler           = glGetUniformLocation(pgm, "normalSampler");
        uniformUseAmbientSampler       = glGetUniformLocation(pgm, "useAmbientSampler");
        uniformUseDiffuseSampler       = glGetUniformLocation(pgm, "useDiffuseSampler");
        uniformUseSpecularSampler      = glGetUniformLocation(pgm, "useSpecularSampler");
        uniformUseSpecularPowerSampler = glGetUniformLocation(pgm, "useSpeculerPowerSampler");
        uniformUseNormalSampler        = glGetUniformLocation(pgm, "useNormalSampler");
        uniformRgbSpecularSampler      = glGetUniformLocation(pgm, "rgbSpecularSampler");
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
        glUniform3fv(      uniformAmbient,          1,           glm::value_ptr(self->ambient));
        glUniform3fv(      uniformDiffuse,          1,           glm::value_ptr(self->diffuse));
        glUniform3fv(      uniformSpecular,         1,           glm::value_ptr(self->specular));
        glUniform1f(       uniformSpecularPower,                 self->specularPower);
        glUniform1i(       uniformAmbientSampler,                0);
        glUniform1i(       uniformDiffuseSampler,                1);
        glUniform1i(       uniformSpecularSampler,               2);
        glUniform1i(       uniformSpecularPowerSampler,          3);
        glUniform1i(       uniformNormalSampler,                 4);
        glUniform1i(       uniformUseAmbientSampler,             self->useAmbientSampler);
        glUniform1i(       uniformUseDiffuseSampler,             self->useDiffuseSampler);
        glUniform1i(       uniformUseSpecularSampler,            self->useSpecularSampler);
        glUniform1i(       uniformUseSpecularPowerSampler,       self->useSpecularPowerSampler);
        glUniform1i(       uniformUseNormalSampler,              self->useNormalSampler);
        glUniform1i(       uniformRgbSpecularSampler,            self->rgbSpecularSampler);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    OpenGLPhongShader* self;
    GLuint pgm;
    GLint uniformProjectionMatrix;
    GLint uniformViewMatrix;
    GLint uniformModelMatrix;
    GLint uniformNormalMatrix;
    GLint uniformCameraPosition;
    GLint uniformSunDirection;
    GLint uniformAmbient;
    GLint uniformDiffuse;
    GLint uniformSpecular;
    GLint uniformSpecularPower;
    GLint uniformAmbientSampler;
    GLint uniformDiffuseSampler;
    GLint uniformSpecularSampler;
    GLint uniformSpecularPowerSampler;
    GLint uniformNormalSampler;
    GLint uniformUseAmbientSampler;
    GLint uniformUseDiffuseSampler;
    GLint uniformUseSpecularSampler;
    GLint uniformUseSpecularPowerSampler;
    GLint uniformUseNormalSampler;
    GLint uniformRgbSpecularSampler;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLPhongShader::OpenGLPhongShader()
    : impl(std::make_shared<Impl>(this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLPhongShader::use()
{ impl->use(); }

} // namespace rasperi
} // namespace kuu
