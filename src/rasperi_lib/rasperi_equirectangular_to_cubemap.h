/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::EquirectangularToCubemap class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include "rasperi_texture_cube.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class EquirectangularToCubemap
{
public:
    EquirectangularToCubemap(int size = 128);

    TextureCube<double, 4> run(const Texture2D<double, 4>& e);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
