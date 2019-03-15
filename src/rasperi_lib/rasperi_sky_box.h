/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::SkyBox class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include "rasperi_texture_cube.h"

namespace kuu
{
namespace rasperi
{

class Framebuffer;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class SkyBox
{
public:
    SkyBox();

    void run(const TextureCube<double, 4>& sky,
             const glm::dmat4& camera,
             const glm::ivec2& viewportSize,
             Framebuffer& framebuffer);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
