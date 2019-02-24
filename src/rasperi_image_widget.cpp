/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::ImageWidget class.
 * ---------------------------------------------------------------- */

#include "rasperi_image_widget.h"
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

    QPainter p(this);
    p.drawImage(0, 0, impl->image);
}

} // namespace rasperi
} // namespace kuu
