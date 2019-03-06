/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::ImageWidget class.
 * ---------------------------------------------------------------- */

#include "rasperi_image_widget.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include "rasperi_camera_controller.h"
#include "rasperi_controller.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct ImageWidget::Impl
{
    void createBgImage(int w, int h)
    {
        int cellSize = 40;
        bgImage = QImage(w, h, QImage::Format_ARGB32);
        QPainter bgPainter(&bgImage);
        int i = 0;
        for (int y = 0; y < h; y += cellSize)
        {
            i = (i % 2 == 0) ? 1 : 0;

            for (int x = 0; x < w;  x += cellSize)
            {
                QRect r(x, y, cellSize, cellSize);
                bgPainter.setPen(Qt::transparent);
                bgPainter.setBrush((i % 2 == 0) ? QColor("#DCDCDC") : Qt::white);
                bgPainter.drawRect(r);
                i++;
            }
        }
    }

    Controller* controller;
    QImage image;
    QImage bgImage;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ImageWidget::ImageWidget(Controller* controller, QWidget* parent)
    : QWidget(parent)
    , impl(std::make_shared<Impl>())
{
    impl->controller = controller;
    impl->createBgImage(width(), height());
    setFocusPolicy(Qt::WheelFocus);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::setImage(const QImage& image)
{
    impl->image = image;
    repaint();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage ImageWidget::bgImage() const
{ return impl->bgImage; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::resizeEvent(QResizeEvent* /*e*/)
{
    impl->controller->setImageSize(width(), height());
    impl->createBgImage(width(), height());
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    QPainter p(this);
    //p.drawImage(0, 0, impl->bgImage);
    p.setBrush(QColor::fromRgbF(0.2, 0.2, 0.2, 1.0));
    p.drawRect(rect());
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(0, 0, impl->image);
    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.setBrush(Qt::transparent);
    p.drawRect(impl->image.rect());
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    impl->controller->cameraController()->setKeyPress(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::keyReleaseEvent(QKeyEvent *e)
{
    impl->controller->cameraController()->setKeyRelease(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::mousePressEvent(QMouseEvent* e)
{
    impl->controller->cameraController()->setMousePress(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::mouseMoveEvent(QMouseEvent* e)
{
    impl->controller->cameraController()->setMouseMove(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::mouseReleaseEvent(QMouseEvent* e)
{
    impl->controller->cameraController()->setMouseRelease(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImageWidget::wheelEvent(QWheelEvent* e)
{
    impl->controller->cameraController()->setWheel(e);
}

} // namespace rasperi
} // namespace kuu
