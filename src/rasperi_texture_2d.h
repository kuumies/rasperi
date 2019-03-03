/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Texture2D class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtGui/QImage>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
template<typename T, int C>
class Texture2D
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    static const int MAGIC_NUMBER = 0xDADCAC;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Texture2D(int width = 0,
              int height = 0,
              const std::vector<T>& pixels = std::vector<T>())
        : d(std::make_shared<Data>())
    {
        d->width  = width;
        d->height = height;
        if (pixels.size())
            d->pixels = pixels;
        else
            d->pixels.resize(width * height * C);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool isNull() const
    { return d->width == 0 || d->height == 0; }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    QImage toQImage() const
    {
        if (C == 1)
            return QImage(d->pixels.data(), d->width, d->height, QImage::Format_Grayscale8);
        if (C == 4)
            return QImage(d->pixels.data(), d->width, d->height, QImage::Format_RGB32);
        return QImage();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool setPixel(double x, double y, std::array<T, C>& pixel)
    {
        int px = int(std::floor(x * double(d->width  - 1)));
        int py = int(std::floor(y * double(d->height - 1)));
        return setPixel(px, py, pixel);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool setPixel(int x, int y, std::array<T, C>& pixel)
    {
        if (x < 0 || x >= d->width)
            return false;
        if (y < 0 || y >= d->height)
            return false;

        for (int i = 0; i < C; ++i)
            d->pixels[y * d->width * C + C * x + i] = pixel[i];

        return true;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::array<T, C> pixel(double x, double y) const
    {
        int px = int(std::floor(x * double(d->width  - 1)));
        int py = int(std::floor(y * double(d->height - 1)));
        return pixel(px, py);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::array<T, C> pixel(int x, int y) const
    {
        if (x < 0 || x >= d->width)
            return {};
        if (y < 0 || y >= d->height)
            return {};

        std::array<T, C> out;
        for (int i = 0; i < C; ++i)
            out[i] = d->pixels[y * d->width * C + C * x + i];
        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool write(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
            return false;

        const int byteCount = int(d->pixels.size()) * sizeof(T);

        QDataStream ds(&file);
        ds << MAGIC_NUMBER;
        ds << d->width;
        ds << d->height;
        ds << C;
        ds << byteCount;
        char* data = reinterpret_cast<char*>(d->pixels.data());
        ds.writeRawData(data, byteCount);
        return true;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool read(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
            return false;

        QDataStream ds(&file);

        int magic = 0;
        ds >> magic;
        if (magic != MAGIC_NUMBER)
            return false;

        int w = 0;
        ds >> w;
        int h = 0;
        ds >> h;

        int c = 0;
        ds >> c;
        if (c != C)
            return false;
        int byteCount = 0;
        ds >> byteCount;

        d->pixels.resize(byteCount / sizeof(T));

        char* data = reinterpret_cast<char*>(d->pixels.data());
        if (ds.readRawData(data, byteCount) == -1)
            return false;

        d->width  = w;
        d->height = h;

        return true;
    }

private:
    friend class MipmapGenerator;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    struct Data
    {
        int width;
        int height;
        std::vector<T> pixels;
        std::vector<Texture2D<T, C>> mipmaps;
    };

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::shared_ptr<Data> d;
};

} // namespace rasperi
} // namespace kuu
