/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::ImageWidget class.
 * ---------------------------------------------------------------- */

#include "rasperi_image_widget.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct ImageWidget::Impl
{
    QImage image;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ImageWidget::ImageWidget(QWidget* parent)
    : QWidget(parent)
    , impl(std::make_shared<Impl>())
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::setImage(const QImage& image)
{
    impl->image = image;
    repaint();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    int cellSize = 40;
    QImage bgImage(width(), height(), QImage::Format_ARGB32);
    QPainter bgPainter(&bgImage);
    int i = 0;
    for (int y = 0; y < height(); y += cellSize)
    {
        i = (i % 2 == 0) ? 1 : 0;

        for (int x = 0; x < width();  x += cellSize)
        {
            QRect r(x, y, cellSize, cellSize);
            bgPainter.setPen(Qt::transparent);
            bgPainter.setBrush((i % 2 == 0) ? QColor("#DCDCDC") : Qt::white);
            bgPainter.drawRect(r);
            i++;
        }
    }

    QPainter p(this);
    p.drawImage(0, 0, bgImage);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(0, 0, impl->image);
    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.setBrush(Qt::transparent);
    p.drawRect(impl->image.rect());
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
}

} // namespace rasperi
} // namespace kuu
