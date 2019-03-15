/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::PbrIblIrradiance class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include "rasperi_texture_cube.h"

class QDir;
class QImage;

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class PbrIblIrradiance
{
public:
    PbrIblIrradiance(int size = 128);

    bool read(const QDir& dir);
    bool write(const QDir& dir);

    void run(const TextureCube<double, 4>& bgCube);

    TextureCube<double, 4> irradianceCubemap;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
