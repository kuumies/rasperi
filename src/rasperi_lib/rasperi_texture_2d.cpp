/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::Texture2D class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_texture_2d.h"

namespace kuu
{
namespace rasperi
{

/* ------------------------------------------------------------ *
   See "Picture File Format (.pic suffix)" from
   http://radsite.lbl.gov/radiance/refer/filefmts.pdf
   Search google with "adaptive run-length encoded" to find
   "Graphics Gems II" page 89
 * ------------------------------------------------------------ */
Texture2D<double, 4> readHdr(const QString& filepath)
{
    if (!QFile::exists(filepath))
    {
        qDebug() << __FUNCTION__ << "file does not exits" << filepath;
        return Texture2D<double, 4>();
    }

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << __FUNCTION__ << "failed to open file" << filepath;
        return Texture2D<double, 4>();
    }

    // Read header. An empty line indicates the end of it. Header
    // always starts with identified "#?RADIANCE"
    QByteArray id = file.readLine();
    if (QString::fromLocal8Bit(id) != "#?RADIANCE\n")
    {
        qDebug() << __FUNCTION__ << "ID is not valid" << filepath;
        return Texture2D<double, 4>();
    }

    while (!file.atEnd())
    {
        QByteArray option = file.readLine();
        QString optionStr = QString::fromLocal8Bit(option);
        if (optionStr == "\n") // header end
            break;
        if (option.at(0) == '#') // comment
            continue;

        QStringList parts = QString::fromLocal8Bit(option).split("=");
        if (parts.size() != 2)
            continue;
        QString name  = parts[0];
        QString value = parts[1];
        //qDebug() << name << value;
        if (name == "FORMAT")
        {
            if (value != "32-bit_rle_rgbe\n")
            {
                qDebug() << __FUNCTION__ << "Format is not 32-bit RLE RGBE" << filepath;
                return Texture2D<double, 4>();
            }
        }
    }

    // Read coodinate axes
    QByteArray axes = file.readLine();
    QString axesStr = QString::fromLocal8Bit(axes);

    // Decode RLE data.
    // [2][2][run][values][run][values][run][values]...
    std::vector<std::vector<uchar>> scanlines;
    while(!file.atEnd())
    {
        // Read scanline indicator
        std::array<uchar, 2> indicator;
        file.read(reinterpret_cast<char*>(indicator.data()), 2);
        if (indicator[0] != 2 || indicator[1] != 2)
        {
            qDebug() << __FUNCTION__ << "Invalid scanline"
                     << QString::number(indicator[0], 16)
                     << QString::number(indicator[1], 16);
            return Texture2D<double, 4>();
        }

        // Read scanline length
        std::array<uchar, 2> length;
        file.read(reinterpret_cast<char*>(length.data()), 2);
        std::swap(length[0], length[1]);
        uint16_t width = *reinterpret_cast<uint16_t*>(length.data());

        // First is whole r scanline, then whole g and then b and then e
        int read = 0;
        std::vector<uchar> scanline;
        while(read < width * 4)
        {
            uchar run = 0;
            file.read(reinterpret_cast<char*>(&run), 1);
           if (run > 128)
            {
                // run
                int c = run - 128;
                uchar data;
                file.read(reinterpret_cast<char*>(&data), 1);
                for (int j = 0; j < c; ++j)
                    scanline.push_back(data);
                read += c;

            }
            else
            {
                // dump
                uchar l = run;
                std::vector<uchar> data;
                data.resize(l);
                file.read(reinterpret_cast<char*>(data.data()), l);
                scanline.insert(scanline.end(), data.begin(), data.end());
                read += l;
            }
        }

        scanlines.push_back(scanline);
    }

    if (scanlines.empty())
    {
        std::cerr << "No scanlines" << std::endl;
        return Texture2D<double, 4>();
    }

    int imgWidth  = int(scanlines[0].size()) / 4;
    int imgHeight = int(scanlines.size());
    std::vector<double> pixels;

    for (const std::vector<uchar>& scanline : scanlines)
    {
        const size_t padding = scanline.size() / 4;
        for (size_t i = 0; i < padding; ++i)
        {
            uchar r = scanline[              i];
            uchar g = scanline[1 * padding + i];
            uchar b = scanline[2 * padding + i];
            uchar e = scanline[3 * padding + i];

            double exponent = std::ldexp(1.0, int(e - 128 + 8));
            pixels.push_back(double((r + 0.5) * exponent) * 0.0001);
            pixels.push_back(double((g + 0.5) * exponent) * 0.0001);
            pixels.push_back(double((b + 0.5) * exponent) * 0.0001);
            pixels.push_back(1.0); // alpha
        }
    }

    return Texture2D<double, 4>(imgWidth, imgHeight, pixels);
}

} // namespace rasperi
} // namespace kuu
