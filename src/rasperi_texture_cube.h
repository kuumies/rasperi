/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::TextureCube class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <array>
#include <memory>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include "rasperi_texture_2d.h"
#include "rasperi_texture_mipmaps.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
template<typename T, int C>
class TextureCube
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    static const int MAGIC_NUMBER = 0xCACCAC;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    TextureCube(int width = 0, int height = 0)
        : d(std::make_shared<Data>())
    {
        d->width  = width;
        d->height = height;
        for (size_t i = 0; i < d->faces.size(); ++i)
            d->faces[i] = Texture2D<T, C>(d->width, d->height);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool isNull() const
    { return d->width == 0 || d->height == 0; }


    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Texture2D<T, C>& face(size_t f, size_t mipmap = 0) const
    { return d->faces[f].mipmap(mipmap); }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool generateMipmaps() const
    {
        MipmapGenerator mipmapGenerator;

        bool ok = true;
        for (size_t f = 0; f < d->faces.size(); ++f)
            ok &= mipmapGenerator.generate(d->faces[f]);
        return ok;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    int mipmapCount() const
    { return d->faces[0].mipmapCount(); }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    QImage toQImage(int mipmap = -1) const
    {
        QImage out;
        if (mipmap == -1)
        {
            out = QImage(d->width * 4, d->height * 3, QImage::Format_RGB32);
            out.fill(0);
            QPainter p(&out);

            const int w = d->width;
            const int h = d->height;
            const QRect sourceRect(0, 0, w, h);
            p.drawImage(QRect(    w,     0, w, h), d->faces[0].toQImage(), sourceRect); // +Y
            p.drawImage(QRect(    0,     h, w, h), d->faces[1].toQImage(), sourceRect); // -X
            p.drawImage(QRect(    w,     h, w, h), d->faces[2].toQImage(), sourceRect); // +Z
            p.drawImage(QRect(2 * w,     h, w, h), d->faces[3].toQImage(), sourceRect); // +X
            p.drawImage(QRect(3 * w,     h, w, h), d->faces[4].toQImage(), sourceRect); // -Z
            p.drawImage(QRect(    w, 2 * h, w, h), d->faces[5].toQImage(), sourceRect); // -Z
        }
        else
        {
            const int w = d->width  >> (mipmap + 1);
            const int h = d->height >> (mipmap + 1);
            const QRect sourceRect(0, 0, w, h);

            out = QImage(w * 4, h * 3, QImage::Format_RGB32);
            out.fill(0);
            QPainter p(&out);
            p.drawImage(QRect(    w,     0, w, h), d->faces[0].mipmap(mipmap).toQImage(), sourceRect); // +Y
            p.drawImage(QRect(    0,     h, w, h), d->faces[1].mipmap(mipmap).toQImage(), sourceRect); // -X
            p.drawImage(QRect(    w,     h, w, h), d->faces[2].mipmap(mipmap).toQImage(), sourceRect); // +Z
            p.drawImage(QRect(2 * w,     h, w, h), d->faces[3].mipmap(mipmap).toQImage(), sourceRect); // +X
            p.drawImage(QRect(3 * w,     h, w, h), d->faces[4].mipmap(mipmap).toQImage(), sourceRect); // -Z
            p.drawImage(QRect(    w, 2 * h, w, h), d->faces[5].mipmap(mipmap).toQImage(), sourceRect); // -Z
        }

        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool write(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
            return false;

        QDataStream ds(&file);
        ds << MAGIC_NUMBER;
        ds << d->width;
        ds << d->height;
        for (size_t f = 0; f < 6; ++f)
            if (!d->faces[f].write(ds))
                return false;

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

        for (size_t f = 0; f < 6; ++f)
            if (!d->faces[f].read(ds))
                return false;

        d->width  = w;
        d->height = h;

        return true;
    }

private:

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    struct Data
    {
        int width;
        int height;
        std::array<Texture2D<T, C>, 6> faces;
    };

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::shared_ptr<Data> d;
};

} // namespace rasperi
} // namespace kuu
