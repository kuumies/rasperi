/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::OpenGLWidget class.
 * ---------------------------------------------------------------- */

#include "rasperi_opengl_widget.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include "rasperi_camera_controller.h"
#include "rasperi_controller.h"
#include "rasperi_opengl_reference_rasterizer/rasperi_opengl_reference_rasterizer.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLWidget::Impl
{
    Impl()
        : rasterizer(512, 512)
    {}

    Controller* controller;
    OpenGLReferenceRasterizer rasterizer;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLWidget::OpenGLWidget(Controller* controller, QWidget* parent)
    : QOpenGLWidget(parent)
    , impl(std::make_shared<Impl>())
{
    impl->controller = controller;
    setFocusPolicy(Qt::WheelFocus);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLReferenceRasterizer& OpenGLWidget::rasterizer()
{ return impl->rasterizer; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::initializeGL()
{
    if (gladLoadGL() == 0)
        throw std::runtime_error("Failed to initialize OpenGL 3.3");
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::resizeGL(int w, int h)
{
    impl->rasterizer = OpenGLReferenceRasterizer(w, h);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::paintGL()
{
    impl->rasterizer.run(defaultFramebufferObject());
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    impl->controller->cameraController()->setKeyPress(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::keyReleaseEvent(QKeyEvent *e)
{
    impl->controller->cameraController()->setKeyRelease(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::mousePressEvent(QMouseEvent* e)
{
    impl->controller->cameraController()->setMousePress(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::mouseMoveEvent(QMouseEvent* e)
{
    impl->controller->cameraController()->setMouseMove(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::mouseReleaseEvent(QMouseEvent* e)
{
    impl->controller->cameraController()->setMouseRelease(e);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::wheelEvent(QWheelEvent* e)
{
    impl->controller->cameraController()->setWheel(e);
}

} // namespace rasperi
} // namespace kuu
