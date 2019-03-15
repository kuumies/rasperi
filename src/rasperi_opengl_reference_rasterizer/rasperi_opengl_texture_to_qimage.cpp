/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::opengl_texture_to_qimage namespace.
 * ---------------------------------------------------------------- */

#include "rasperi_opengl_shader_loader.h"
#include <array>
#include <QtGui/QImage>
#include <QtGui/QPainter>

namespace kuu
{
namespace rasperi
{
namespace opengl_texture_to_qimage
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage readRgbaTexture(GLuint tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);

    GLint w, h;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

    QImage image(w, h, QImage::Format_RGBA8888);
    glGetTexImage(GL_TEXTURE_2D, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE,
                  image.bits());
    glBindTexture(GL_TEXTURE_2D, 0);

    QImage img = image.convertToFormat(QImage::Format_ARGB32);
    return img.rgbSwapped();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage textureCube(GLuint tex, GLint level)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    GLint w = 0, h = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level,
                             GL_TEXTURE_WIDTH,  &w);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level,
                             GL_TEXTURE_HEIGHT, &h);

    // Read faces
    std::array<QImage, 6> faces;
    for (size_t face = 0; face < 6; ++face)
    {
        QImage image(w, h, QImage::Format_RGBA8888);
        glGetTexImage(GLenum(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face), level,
                      GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glBindTexture(GL_TEXTURE_2D, 0);

        faces[face] = image.convertToFormat(QImage::Format_ARGB32);
    }

    QImage output(w * 4, h * 3, QImage::Format_RGB32);
    output.fill(0);

    QPainter p(&output);
    for (size_t face = 0; face < 6; ++face)
    {
        QImage faceTex = faces[face];

        int x = 0, y = 0;
        switch(face)
        {
            case 0:
                x = 2 * w;
                y = 1 * h;
                break;

            case 1:
                x = 0 * w;
                y = 1 * h;
                break;

            case 2:
                x = 1 * w;
                y = 0 * h;
                break;

            case 3:
                x = 1 * w;
                y = 2 * h;
                break;

            case 4:
                x = 1 * w;
                y = 1 * h;
                break;

            case 5:
                x = 3 * w;
                y = 1 * h;
                break;
        }

        p.drawImage(x, y, faceTex);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return output;
}

} // namespace opengl_texture_to_qimage
} // namespace rasperi
} // namespace kuu
