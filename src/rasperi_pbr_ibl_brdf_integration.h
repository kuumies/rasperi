/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::PbrIblBrdfIntegration class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <QtCore/QDir>
#include "rasperi_texture_2d.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class PbrIblBrdfIntegration
{
public:
    PbrIblBrdfIntegration(int size = 128);

    bool read(const QDir& dir);
    bool write(const QDir& dir);

    void run();

    Texture2D<double, 2> brdfIntegration2dMap;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
