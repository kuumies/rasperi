/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::PbrIblPrefilter class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include "rasperi_texture_cube.h"

class QDir;

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class PbrIblPrefilter
{
public:
    PbrIblPrefilter(int size = 128);

    bool read(const QDir& dir);
    bool write(const QDir& dir);

    void run(const QImage& bgMap);

    TextureCube<double, 4> prefilterCubemap;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
