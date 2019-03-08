/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLPbrTexture class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_pbr_textures.h"
#include <QDebug>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLPbrTexture::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(OpenGLPbrTexture* self, const Material& material)
        : self(self)
    {
        qDebug() << material.pbr.albedoSampler.isValid()
                 << material.pbr.roughnessSampler.isValid()
                 << material.pbr.metalnessSampler.isValid()
                 << material.pbr.aoSampler.isValid()
                 << material.normalSampler.isValid();

        if (material.pbr.albedoSampler.isValid())
            texAlbedo = createTexture(material.pbr.albedoSampler.map(),
                                      material.pbr.albedoSampler.linearizeGamma());

        if (material.pbr.roughnessSampler.isValid())
            texRoughness = createTexture(material.pbr.roughnessSampler.map(),
                                         material.pbr.roughnessSampler.linearizeGamma());

        if (material.pbr.metalnessSampler.isValid())
            texMetalness = createTexture(material.pbr.metalnessSampler.map(),
                                         material.pbr.metalnessSampler.linearizeGamma());

        if (material.pbr.aoSampler.isValid())
            texAo = createTexture(material.pbr.aoSampler.map(),
                                  material.pbr.aoSampler.linearizeGamma());

        if (material.normalSampler.isValid())
            texNormal = createTexture(material.normalSampler.map(),
                                      material.normalSampler.linearizeGamma());
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    ~Impl()
    {
        glDeleteTextures(1, &texAlbedo);
        glDeleteTextures(1, &texRoughness);
        glDeleteTextures(1, &texMetalness);
        glDeleteTextures(1, &texAo);
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
        glBindTexture(GL_TEXTURE_2D, texAlbedo);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texRoughness);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texMetalness);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texAo);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, texNormal);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    OpenGLPbrTexture* self;
    GLuint texAlbedo;
    GLuint texRoughness;
    GLuint texMetalness;
    GLuint texAo;
    GLuint texNormal;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLPbrTexture::OpenGLPbrTexture(const Material& material)
    : impl(std::make_shared<Impl>(this, material))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLPbrTexture::bind()
{ impl->bind(); }

} // namespace rasperi
} // namespace kuu
