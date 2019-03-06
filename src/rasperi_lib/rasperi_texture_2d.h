/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Texture2D class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <iostream>
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
    int width() const
    { return d->width; }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    int height() const
    { return d->height; }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    QImage toQImage() const
    {
        // See https://en.cppreference.com/w/cpp/language/typeid
        const std::type_info& ti1 = typeid(T);
        const std::type_info& ti2 = typeid(double);
        const std::type_info& ti3 = typeid(float);

        bool floatingPoint = ti1.hash_code() == ti2.hash_code() ||
                             ti1.hash_code() == ti3.hash_code();

        if (floatingPoint)
        {
            int channels = C;
            if (channels == 2 || channels == 3)
                channels = 4;
            std::vector<uchar> data;
            for (int y = 0; y < d->height; ++y)
            for (int x = 0; x < d->width;  ++x)
            {
                for (int c = 0; c < C; ++c)
                {
                    T v = d->pixels[y * d->width * C + C * x + c];
                    //v = v / (v + T(1.0)); // tone mapping HDR -> SDR
                    data.push_back(qRound(v * 255.0));
                }
                if (channels != C)
                {
                    if (C == 2)
                        data.push_back(0); // b
                    data.push_back(255);   // a
                }
            }

            switch(channels)
            {
                case 1: return QImage(data.data(), d->width, d->height, QImage::Format_Grayscale8).copy();
                case 4: return QImage(data.data(), d->width, d->height, QImage::Format_ARGB32).rgbSwapped().copy();
                default: break;
            }
        }
        else
        {
            uchar* data = reinterpret_cast<uchar*>(d->pixels.data());
            switch(C)
            {
                case 1: return QImage(data, d->width, d->height, QImage::Format_Grayscale8);
                case 4: return QImage(data, d->width, d->height, QImage::Format_ARGB32).rgbSwapped();
                default: break;
            }

        }
        return QImage();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void clear(const std::array<T, C>& value)
    {
        for (size_t y = 0; y < d->height; y++)
        for (size_t x = 0; x < d->width;  x++)
        for (int i = 0; i < C; ++i)
            d->pixels[y * d->width * C + C * x + i] = value[i];
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool setPixel(double x, double y, std::array<T, C>& pixel)
    {
        int px = int(std::floor(x * double(d->width  - 1)));
        int py = int(std::floor(y * double(d->height - 1)));

//        std::cout << px << ", " << py << ", "
//                  << pixel[0] << ", " << pixel[1]
//                  << std::endl;
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
    std::vector<T>& pixels()
    { return d->pixels; }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    int mipmapCount() const
    { return int(d->mipmaps.size()); }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Texture2D<T, C>& mipmap(size_t index)
    {
        if (index == 0)
            return *this;
        else
            return d->mipmaps[index - 1];
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool write(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
            return false;

        QDataStream ds(&file);
        return write(ds);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool write(QDataStream& ds)
    {
        const int byteCount = int(d->pixels.size()) * sizeof(T);

        ds << MAGIC_NUMBER;
        ds << d->width;
        ds << d->height;
        ds << C;
        ds << byteCount;
        char* data = reinterpret_cast<char*>(d->pixels.data());
        ds.writeRawData(data, byteCount);

        ds << int(d->mipmaps.size());
        for (int i = 0; i < d->mipmaps.size(); ++i)
            d->mipmaps[i].write(ds);

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
        return read(ds);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool read(QDataStream& ds)
    {
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

        int mipmapCount = 0;
        ds >> mipmapCount;
        d->mipmaps.resize(mipmapCount);
        for (int i = 0; i < mipmapCount; ++i)
            d->mipmaps[i].read(ds);

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
