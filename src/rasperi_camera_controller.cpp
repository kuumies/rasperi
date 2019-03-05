/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Rasperi::CameraController class.
 * -----------------------------------------------------------------*/

#include "rasperi_camera_controller.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include "rasperi_camera.h"
#include "rasperi_controller.h"

namespace kuu
{
namespace rasperi
{

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
struct CameraController::Impl
{
    Impl(Controller* controller)
        : controller(controller)
    {}

    glm::dvec3 position  = glm::dvec3();
    glm::dquat rotation  = glm::dquat();
    double pitch         = 0.0;
    double yaw           = 0.0;
    double zoomAmount    = 0.5;

    bool rotate          = false;
    QPointF rotatePos    = QPointF();
    bool translate       = false;
    QPointF translatePos = QPointF();

    std::map<int, bool> keyDownMap;

    Controller* controller;
};

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
CameraController::CameraController(Controller* controller)
    : impl(std::make_shared<Impl>(controller))
{}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
void CameraController::setZoomAmount(double amount)
{ impl->zoomAmount = amount; }

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
//void CameraController::update(float /*elapsed*/)
//{
//    glm::dvec3 moveDir;
//    if (impl->keyDownMap[Qt::Key_W] ||
//        impl->keyDownMap[Qt::Key_Up])
//    {
//        moveDir.z = -1.0;
//    }
//    if (impl->keyDownMap[Qt::Key_S] ||
//        impl->keyDownMap[Qt::Key_Down])
//    {
//        moveDir.z = 1.0;
//    }
//    if (impl->keyDownMap[Qt::Key_A] ||
//        impl->keyDownMap[Qt::Key_Left])
//    {
//        moveDir.x = -1.0;
//    }
//    if (impl->keyDownMap[Qt::Key_D] ||
//        impl->keyDownMap[Qt::Key_Right])
//    {
//        moveDir.x = 1.0;
//    }

//    moveDir = glm::dvec3(glm::inverse(impl->camera->viewMatrix()) * glm::dvec4(moveDir, 0.0));

//    impl->position += moveDir * 0.5;

//    impl->camera->position = glm::mix(  impl->camera->position, impl->position, 0.1);
//    impl->camera->rotation = glm::slerp(impl->camera->rotation, impl->rotation, 0.1);
//}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
void CameraController::setMousePress(QMouseEvent* e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        impl->translate    = true;
        impl->translatePos = e->pos();
    }

    if (e->buttons() & Qt::RightButton)
    {
        impl->rotate    = true;
        impl->rotatePos = e->pos();
        impl->controller->rasterize(true);
    }
}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
void CameraController::setMouseMove(QMouseEvent* e)
{
    if (impl->translate)
    {
        const QPointF diff = e->pos() - impl->translatePos;
        impl->translatePos = e->pos();

        glm::dvec4 fc(0.0, 0.0, -1.0, 0.0);
        glm::dvec3 fw = glm::inverse(impl->controller->camera()->viewMatrix()) * fc;


    }

    if (impl->rotate)
    {
        // Get the cursor position difference related to previous
        // move.
        const QPointF diff = e->pos() - impl->rotatePos;
        impl->rotatePos = e->pos();

        double pitch = glm::radians(diff.y()) * 0.15;
        double yaw   = glm::radians(diff.x()) * 0.15;

        impl->yaw   += yaw;
        impl->pitch += pitch;

//        const double halfPi = M_PI_2;
//        if (impl->pitch >= halfPi || impl->pitch <= -halfPi)
//        {
//            pitch = 0.0;
//            impl->pitch = glm::clamp(impl->pitch, -halfPi, halfPi);
//        }

        glm::dquat rotation = impl->controller->camera()->rotation;
        const glm::dvec3 pitchAxis = glm::vec3(1.0, 0.0, 0.0);
        const glm::dvec3 yawAxis   = rotation *  glm::dvec3(0.0, 1.0, 0.0);

        rotation = glm::angleAxis(yaw,   yawAxis)   * rotation;
        rotation = glm::angleAxis(pitch, pitchAxis) * rotation;
        impl->controller->camera()->rotation = rotation;
        impl->controller->rasterize(true);
    }
}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
void CameraController::setMouseRelease(QMouseEvent* /*e*/)
{
    impl->rotate       = false;
    impl->rotatePos    = QPointF();
    impl->translate    = false;
    impl->translatePos = QPointF();
    impl->controller->rasterize(true);
}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
void CameraController::setWheel(QWheelEvent* e)
{
    double amount = e->delta() > 0 ? -impl->zoomAmount : impl->zoomAmount;
    impl->controller->camera()->viewDistance += amount;
    impl->controller->rasterize(true);
}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
void CameraController::setKeyPress(QKeyEvent* e)
{
    glm::dvec3 moveDir;
    if (e->key() == Qt::Key_W)
    {
        moveDir.z = -1.0;
    }
    if (e->key() == Qt::Key_S)
    {
        moveDir.z = 1.0;
    }
    if (e->key() == Qt::Key_A)
    {
        moveDir.x = -1.0;
    }
    if (e->key() == Qt::Key_D)
    {
        moveDir.x = 1.0;
    }

    auto camera = impl->controller->camera();
    moveDir = glm::dvec3(glm::inverse(camera->viewMatrix()) * glm::dvec4(moveDir, 0.0));

    camera->position += moveDir * 0.5;
    impl->controller->rasterize(true);
}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
void CameraController::setKeyRelease(QKeyEvent* e)
{
    impl->keyDownMap[e->key()] = false;
    impl->controller->rasterize(true);
}

} // namespace rapseri
} // namespace kuu
