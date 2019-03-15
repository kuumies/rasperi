/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLSkyboxShader class.
 * ---------------------------------------------------------------- */

#include "rasperi_opengl_sky_box_shader.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "rasperi_opengl_shader_loader.h"

namespace kuu
{
namespace rasperi
{

using namespace opengl_shader_loader;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLSkyBoxShader::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(OpenGLSkyBoxShader* self)
        : self(self)
    {
        pgm = load("shaders/rasperi_opengl_sky_box_shader.vsh",
                   "shaders/rasperi_opengl_sky_box_shader.fsh");

        uniformProjectionMatrix = glGetUniformLocation(pgm, "matrices.projection");
        uniformViewMatrix       = glGetUniformLocation(pgm, "matrices.view");
        uniformMap              = glGetUniformLocation(pgm, "skyboxMap");
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
        glUseProgram(pgm);
        glUniformMatrix4fv(uniformViewMatrix,       1, GL_FALSE, glm::value_ptr(self->viewMatrix));
        glUniformMatrix4fv(uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(self->projectionMatrix));
        glUniform1i(       uniformMap,                           0);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    OpenGLSkyBoxShader* self;
    GLuint pgm;
    GLint uniformProjectionMatrix;
    GLint uniformViewMatrix;
    GLint uniformMap;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLSkyBoxShader::OpenGLSkyBoxShader()
    : impl(std::make_shared<Impl>(this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLSkyBoxShader::use()
{ impl->use(); }

} // namespace rasperi
} // namespace kuu
