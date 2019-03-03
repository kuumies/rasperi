/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::PbrIblIrradiance class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <QtCore/QDir>
#include <QtGui/QImage>
#include "rasperi_texture_2d.h"
#include "rasperi_texture_cube.h"


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

    void run(const QImage& bgMap);

    TextureCube<double, 4> irradianceCubemap;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
