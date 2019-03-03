/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::DoubleXYMap classes.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <array>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <QtCore/QVector>
#include <QtGui/QImage>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class DoubleRg2dMap
{
public:
    const int MAGIC_NUMBER = 0xCACDAD;

    DoubleRg2dMap(int w, int h);

    void set(int x, int y, const glm::dvec2& v);
    glm::dvec2 get(int x, int y) const;

    bool write(const QString& filePath);
    bool read(const QString& filePath);

    QImage toQImage() const;

    int w;
    int h;
    QVector<double> data;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class DoubleRgbCubeMap
{
public:
    const int MAGIC_NUMBER = 0xDADCAC;

    DoubleRgbCubeMap(int faceWidth, int faceHeight);

    void set(size_t face, int x, int y, const glm::dvec3& v);
    glm::dvec3 get(size_t face, int x, int y) const;

    bool write(const QString& filePath);
    bool read(const QString& filePath);

    QImage toQImage() const;

    int w;
    int h;
    std::array<QVector<double>, 6> data;
};

} // namespace rasperi
} // namespace kuu
