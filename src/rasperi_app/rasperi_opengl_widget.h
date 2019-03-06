/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::OpenGLWidget class.
 * ---------------------------------------------------------------- */

#pragma once

#include <glad/glad.h>
#include <QtWidgets/QOpenGLWidget>

namespace kuu
{
namespace rasperi
{

class Controller;
class OpenGLReferenceRasterizer;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OpenGLWidget(
        Controller* controller,
        QWidget* parent = nullptr);

    OpenGLReferenceRasterizer& rasterizer();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
