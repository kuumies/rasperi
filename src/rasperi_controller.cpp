/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#include "rasperi_controller.h"

#include <iostream>
#include <QtCore/QTime>
#include <QtGui/QKeyEvent>
#include <QtGui/QOpenGLDebugLogger>

namespace kuu
{
namespace rasperi
{


/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Controller::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(Controller* self)
        : self(self)
    {}

    Controller* self = nullptr;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Controller::Controller()
    : impl(std::make_shared<Impl>(this))
{}

} // namespace rasperi
} // namespace kuu
