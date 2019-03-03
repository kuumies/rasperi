/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::DoubleXYMap classes.
 * ---------------------------------------------------------------- */
 
#include "rasperi_double_map.h"
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtGui/QPainter>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
DoubleRg2dMap::DoubleRg2dMap(int w, int h)
    : w(w)
    , h(h)
{
    data.resize(w * h * 2);
    data.fill(0.0);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void DoubleRg2dMap::set(int x, int y, const glm::dvec2 &v)
{
    data[y * w + x * 2 + 0] = v.r;
    data[y * w + x * 2 + 1] = v.g;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec2 DoubleRg2dMap::get(int x, int y) const
{
    glm::dvec2 out;
    out.r = data[y * w + x * 2 + 0];
    out.g = data[y * w + x * 2 + 1];
    return out;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool DoubleRg2dMap::write(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream ds(&file);
    ds << MAGIC_NUMBER;
    ds << w;
    ds << h;
    ds << int(data.size());
    ds << data;

    return true;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool DoubleRg2dMap::read(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream ds(&file);

    int magic = 0;
    ds >> magic;
    if (magic != MAGIC_NUMBER)
        return false;

    ds >> w;
    ds >> h;

    int size = 0;
    ds >> size;
    ds >> data;

    return true;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage DoubleRg2dMap::toQImage() const
{
    QImage out(w, h, QImage::Format_RGB32);
    for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
    {
        glm::dvec2 c = get(x, y);
        c = c / (c + glm::dvec2(1.0));
        out.setPixel(x, y, qRgba(qRound(c.r * 255),
                                 qRound(c.g * 255),
                                 0,
                                 255));
    }
    return out;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
DoubleRgbCubeMap::DoubleRgbCubeMap(int w, int h)
    : w(w)
    , h(h)
{
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i].resize(w * h * 3);
        data[i].fill(0.0);
    }
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void DoubleRgbCubeMap::set(size_t face, int x, int y, const glm::dvec3& v)
{
    data[face][y * w + x * 3 + 0] = v.r;
    data[face][y * w + x * 3 + 1] = v.g;
    data[face][y * w + x * 3 + 2] = v.b;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec3 DoubleRgbCubeMap::get(size_t face, int x, int y) const
{
    glm::dvec3 out;
    out.r = data[face][y * w + x * 3 + 0];
    out.g = data[face][y * w + x * 3 + 1];
    out.b = data[face][y * w + x * 3 + 2];
    return out;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool DoubleRgbCubeMap::write(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream ds(&file);
    ds << MAGIC_NUMBER;
    ds << w;
    ds << h;
    ds << int(data.size());
    for (size_t i = 0; i < data.size(); ++i)
    {
        ds << int(i);
        ds << data[i];
    }
    return true;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool DoubleRgbCubeMap::read(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream ds(&file);

    int magic = 0;
    ds >> magic;
    if (magic != MAGIC_NUMBER)
        return false;

    ds >> w;
    ds >> h;

    int size = 0;
    ds >> size;
    if (size != 6)
        return false;
    for (int i = 0; i < 6; ++i)
    {
        int j;
        ds >> j;
        if (i != j)
            return false;
        ds >> data[size_t(j)];
    }

    return true;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage DoubleRgbCubeMap::toQImage() const
{
    std::array<QImage, 6> faces;
    for (size_t i = 0; i < data.size(); ++i)
    {
        QImage face(w, h, QImage::Format_RGB32);
        for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            glm::dvec3 c = get(i, x, y);
            c = c / (c + glm::dvec3(1.0));
            face.setPixel(x, y, qRgba(qRound(c.r * 255),
                                      qRound(c.g * 255),
                                      qRound(c.b * 255),
                                      255));
        }
        faces[i] = face;
    }

    QImage out(4 * w, 3 * h, QImage::Format_RGB32);
    out.fill(0);
    QPainter p(&out);
    p.drawImage(QRect(    w,     0, w, h), faces[0], QRect(0, 0, w, h)); // +Y
    p.drawImage(QRect(    0,     h, w, h), faces[1], QRect(0, 0, w, h)); // -X
    p.drawImage(QRect(    w,     h, w, h), faces[2], QRect(0, 0, w, h)); // +Z
    p.drawImage(QRect(2 * w,     h, w, h), faces[3], QRect(0, 0, w, h)); // +X
    p.drawImage(QRect(3 * w,     h, w, h), faces[4], QRect(0, 0, w, h)); // -Z
    p.drawImage(QRect(    w, 2 * h, w, h), faces[5], QRect(0, 0, w, h)); // -Z
    return out;
}

} // namespace rasperi
} // namespace kuu
