/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::wakusei::Controller class.
 * ---------------------------------------------------------------- */

#include "rasperi_controller.h"

#include <iostream>
#include <QtCore/QTime>
#include <QtGui/QKeyEvent>
#include <QtGui/QOpenGLDebugLogger>

namespace kuu
{
namespace wakusei
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

} // namespace wakusei
} // namespace kuu
