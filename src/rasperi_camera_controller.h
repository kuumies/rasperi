/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::CameraController class.
 * -----------------------------------------------------------------*/

#pragma once

#include <glm/vec4.hpp>
#include <functional>
#include <memory>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace kuu
{
namespace rasperi
{

class Controller;

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
class CameraController
{
public:
    CameraController(Controller* controller);

    void setMousePress(QMouseEvent* e);
    void setMouseMove(QMouseEvent* e);
    void setMouseRelease(QMouseEvent* e);
    void setWheel(QWheelEvent* e);
    void setKeyPress(QKeyEvent* e);
    void setKeyRelease(QKeyEvent* e);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
