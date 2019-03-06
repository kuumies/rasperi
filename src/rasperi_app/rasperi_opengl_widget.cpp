/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::OpenGLWidget class.
 * ---------------------------------------------------------------- */

#include "rasperi_opengl_widget.h"
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
struct OpenGLWidget::Impl
{
    Impl()
    {}

    Controller* controller;
    OpenGLReferenceRasterizer* rasterizer = nullptr;
    OpenGLReferenceRasterizer::Scene scene;
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
void OpenGLWidget::setScene(const OpenGLReferenceRasterizer::Scene& scene)
{ impl->scene = scene; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::initializeGL()
{
    if (gladLoadGL() == 0)
        throw std::runtime_error("Failed to initialize OpenGL 3.3");
    impl->rasterizer = new OpenGLReferenceRasterizer();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::resizeGL(int w, int h)
{
    delete impl->rasterizer;
    impl->rasterizer = new OpenGLReferenceRasterizer();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLWidget::paintGL()
{
//    impl->rasterizer.clear();
//    impl->rasterizer.setNormalMode(OpenGLReferenceRasterizer::NormalMode::Smooth);
//    impl->rasterizer.setViewMatrix(impl->scene.view);
//    impl->rasterizer.setProjectionMatrix(impl->scene.projection);
//    for (Model& model : impl->scene.models)
//    {
//        if (model.transform)
//            impl->rasterizer.setModelMatrix(model.transform->matrix());
//        if (model.material)
//            impl->rasterizer.setMaterial(*model.material);
//        if (filled)
//            impl->rasterizer.drawFilledTriangleMesh(model.mesh.get());
//        else
//            impl->rasterizer.drawEdgeLineTriangleMesh(model.mesh.get());
//    }
    impl->scene.viewport.z = width();
    impl->scene.viewport.w = height();
    impl->rasterizer->run(defaultFramebufferObject(),
                          impl->scene);
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
