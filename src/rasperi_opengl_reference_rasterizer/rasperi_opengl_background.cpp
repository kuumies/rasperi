/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLBackground class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_background.h"
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include "rasperi_opengl_texture_to_qimage.h"


namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLBackground::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(OpenGLBackground* self)
        : self(self)
        , tex(0)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    ~Impl()
    {
        glDeleteTextures(1, &tex);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void create(const QImage& bg)
    {
        const int size = 512;
        QImage bgMapScaled =
            bg.scaled(size, size,
                      Qt::KeepAspectRatioByExpanding);
        bgMapScaled = bgMapScaled.copy(0, 0, size, size);

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
                         GL_RGBA    , size, size, 0,
                         GL_RGBA    , GL_UNSIGNED_BYTE, bgMapScaled.bits());
        }

        bgMapScaled.save("/temp/00_bg_scaled.bmp");
        opengl_texture_to_qimage::textureCube(tex).save("/temp/00_bg.bmp");
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void create(const TextureCube<double, 4>& bg)
    {

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
            std::vector<float> pixels;
            for (const auto& p : bg.face(face).pixels())
                pixels.push_back(float(p));

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
                         GL_RGBA16F, bg.width(), bg.height(), 0,
                         GL_RGBA, GL_FLOAT,
                         pixels.data());
        }

        opengl_texture_to_qimage::textureCube(tex).save("/temp/00_bg_cube.bmp");
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void draw()
    {
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    OpenGLBackground* self;
    GLuint tex;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLBackground::OpenGLBackground(const QImage& bg)
    : impl(std::make_shared<Impl>(this))
{ impl->create(bg); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLBackground::OpenGLBackground(const TextureCube<double, 4> &bg)
    : impl(std::make_shared<Impl>(this))
{ impl->create(bg); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
GLuint OpenGLBackground::tex() const
{ return impl->tex; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLBackground::draw()
{ impl->draw(); }

} // namespace rasperi
} // namespace kuu
