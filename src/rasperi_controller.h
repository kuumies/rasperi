/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::wakusei::Controller class.
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>

namespace kuu
{
namespace wakusei
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Controller
{
public:
    Controller();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace wakusei
} // namespace kuu
