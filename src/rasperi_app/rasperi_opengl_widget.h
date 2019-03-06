/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::OpenGLWidget class.
 * ---------------------------------------------------------------- */

#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <QtWidgets/QOpenGLWidget>
#include "rasperi_opengl_reference_rasterizer/rasperi_opengl_reference_rasterizer.h"

namespace kuu
{
namespace rasperi
{

class Controller;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OpenGLWidget(
        Controller* controller,
        QWidget* parent = nullptr);

    void setScene(const OpenGLReferenceRasterizer::Scene& scene);

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
