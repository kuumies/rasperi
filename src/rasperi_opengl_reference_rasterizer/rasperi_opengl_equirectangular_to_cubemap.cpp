/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::EquirectangularToCubemap class
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_equirectangular_to_cubemap.h"
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

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLEquirectangularToCubemap::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(const int size, OpenGLEquirectangularToCubemap* self)
        : size(size)
        , self(self)
    {
        glGenTextures(1, &self->cubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, self->cubemap);
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
                         GL_RGB16F, size, size, 0,
                         GL_RGB, GL_FLOAT, nullptr);
        }
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    ~Impl()
    {
        glDeleteTextures(1, &self->cubemap);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(const GLuint tex)
    {
        // --------------------------------------------------------
        // Depth renberbuffer framebuffer attachment

        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                              size, size);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // --------------------------------------------------------
        // Framebuffer

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  rbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // --------------------------------------------------------
        // Shader program

        GLuint pgm =
            opengl_shader_loader::load(
                "shaders/rasperi_opengl_equirectangular_to_cubemap.vsh",
                "shaders/rasperi_opengl_equirectangular_to_cubemap.fsh");

        GLint matricesProjectionLoc = glGetUniformLocation(pgm, "matrices.projection");
        GLint matricesViewLoc       = glGetUniformLocation(pgm, "matrices.view");
        GLint textureCubeLoc        = glGetUniformLocation(pgm, "map");

        // ---------------------------------------------------------------
        // Render texture cube faces

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        NdcCubeMesh ndcBox;
        CubeCamera cubeCamera(1.0);

        for (GLenum face = 0; face < 6; ++face)
        {
            glm::mat4 projection = cubeCamera.projectionMatrix;
            glm::mat4 view       = cubeCamera.viewMatrices[face];

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   self->cubemap,
                                   0);
            glViewport(0, 0, size, size);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(pgm);
            glUniform1i(textureCubeLoc, 0);
            glUniformMatrix4fv(matricesViewLoc,       1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(matricesProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

            ndcBox.draw();
        }

        opengl_texture_to_qimage::textureCube(self->cubemap).save("/temp/00_equirect_to_cubemap.bmp");

        glDeleteRenderbuffers(1, &rbo);
        glDeleteFramebuffers(1, &fbo);
        glDeleteProgram(pgm);
    }

    const int size;
    OpenGLEquirectangularToCubemap* self;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLEquirectangularToCubemap::OpenGLEquirectangularToCubemap(int size)
    : impl(std::make_shared<Impl>(size, this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLEquirectangularToCubemap::run(const Texture2D<double, 4>& t)
{
    std::vector<float> pixels;
    for (const auto& p : t.pixels())
        pixels.push_back(float(p));

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, t.width(), t.height(), 0,
                 GL_RGBA, GL_FLOAT, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    opengl_texture_to_qimage::readRgbaTexture(tex).save("/temp/00_equi2d.bmp");

    run(tex);

    glDeleteTextures(1, &tex);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLEquirectangularToCubemap::run(const GLuint tex)
{ impl->run(tex); }

} // namespace rasperi
} // namespace kuu
