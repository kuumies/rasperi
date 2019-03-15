/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::opengl::rasperi::opengl_texture_to_qimage namespace.
 * ---------------------------------------------------------------- */

#pragma once

#include <glad/glad.h>

class QImage;

namespace kuu
{
namespace rasperi
{
namespace opengl_texture_to_qimage
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage readRgbaTexture(GLuint tex);

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage textureCube(GLuint tex, GLint level = 0);

} // namespace opengl_texture_to_qimage
} // namespace rasperi
} // namespace kuu
