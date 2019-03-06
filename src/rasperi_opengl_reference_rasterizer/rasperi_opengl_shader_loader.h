/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::opengl::rasperi::shader_loader namespace.
 * ---------------------------------------------------------------- */

#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>

namespace kuu
{
namespace rasperi
{
namespace opengl_shader_loader
{

GLuint load(const std::string& vshPath,
            const std::string& fshPath);

} // namespace opengl_shader_loader
} // namespace rasperi
} // namespace kuu
