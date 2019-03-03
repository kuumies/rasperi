/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::TextureCube class.
 * ---------------------------------------------------------------- */
 
#pragma once

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
    Texture2D<T, C> face(size_t f) const
    { return d->faces[f]; }

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
    QImage toQImage() const
    {
        QImage out(d->width * 4, d->height * 3, QImage::Format_RGB32);
        out.fill(0);

        const int w = d->width;
        const int h = d->height;
        const QRect sourceRect(0, 0, w, h);

        QPainter p(&out);
        p.drawImage(QRect(    w,     0, w, h), d->faces[0].getQImage(), sourceRect); // +Y
        p.drawImage(QRect(    0,     h, w, h), d->faces[1].getQImage(), sourceRect); // -X
        p.drawImage(QRect(    w,     h, w, h), d->faces[2].getQImage(), sourceRect); // +Z
        p.drawImage(QRect(2 * w,     h, w, h), d->faces[3].getQImage(), sourceRect); // +X
        p.drawImage(QRect(3 * w,     h, w, h), d->faces[4].getQImage(), sourceRect); // -Z
        p.drawImage(QRect(    w, 2 * h, w, h), d->faces[5].getQImage(), sourceRect); // -Z

        return out;
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
