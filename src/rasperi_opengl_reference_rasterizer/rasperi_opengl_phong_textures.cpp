/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLPhongTextures class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_phong_textures.h"
#include <QDebug>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLPhongTexture::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(OpenGLPhongTexture* self, const Material& material)
        : self(self)
    {
        qDebug() << material.phong.ambientSampler.isValid()
                 << material.phong.diffuseSampler.isValid()
                 << material.phong.specularSampler.isValid()
                 << material.phong.specularPowerSampler.isValid()
                 << material.normalSampler.isValid();

        if (material.phong.ambientSampler.isValid())
            texAmbient = createTexture(material.phong.ambientSampler.map(), false);
        if (material.phong.diffuseSampler.isValid())
            texDiffuse = createTexture(material.phong.diffuseSampler.map(), true);
        if (material.phong.specularSampler.isValid())
            texSpecular = createTexture(material.phong.specularSampler.map(), false);
        if (material.phong.specularPowerSampler.isValid())
            texSpecularPower = createTexture(material.phong.specularPowerSampler.map(), false);
        if (material.normalSampler.isValid())
            texNormal = createTexture(material.normalSampler.map(), false);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    ~Impl()
    {
        glDeleteTextures(1, &texAmbient);
        glDeleteTextures(1, &texDiffuse);
        glDeleteTextures(1, &texSpecular);
        glDeleteTextures(1, &texSpecularPower);
        glDeleteTextures(1, &texNormal);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    GLuint createTexture(const QImage& image,
                         const bool hasGamma)
    {
        GLenum format = 0, internalFormat = 0;
        switch(image.format())
        {
            case QImage::Format_ARGB32:
                format = GL_BGRA;
                internalFormat = hasGamma ? GL_SRGB_ALPHA : GL_RGBA;
                break;

            case QImage::Format_RGB32:
                format = GL_BGRA;
                internalFormat = hasGamma ? GL_SRGB_ALPHA : GL_RGBA;
                break;

            case QImage::Format_Grayscale8:
                format = GL_RED;
                internalFormat = GL_RED;
                break;

            default:
                std::cerr << __FUNCTION__
                          << ": unsupported image format"
                          << std::endl;
                return 0;
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D,  tex);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width(), image.height(),
                     0, format, GL_UNSIGNED_BYTE, image.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D,  0);
        return tex;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void bind()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texAmbient);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texDiffuse);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texSpecular);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texSpecularPower);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, texNormal);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    OpenGLPhongTexture* self;
    GLuint texAmbient;
    GLuint texDiffuse;
    GLuint texSpecular;
    GLuint texSpecularPower;
    GLuint texNormal;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLPhongTexture::OpenGLPhongTexture(const Material& material)
    : impl(std::make_shared<Impl>(this, material))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLPhongTexture::bind()
{ impl->bind(); }

} // namespace rasperi
} // namespace kuu
