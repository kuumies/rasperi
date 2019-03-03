/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::PbrIbl class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <QtCore/QDir>
#include <QtGui/QImage>
#include "rasperi_double_map.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class PbrIbl
{
public:
    PbrIbl(int size = 128);

    bool read(const QDir& dir);
    bool write(const QDir& dir);

    void run(const QImage& bgMap);

    DoubleRgbCubeMap irradiance;
    DoubleRgbCubeMap prefilter;
    DoubleRg2dMap brdfIntegration;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
