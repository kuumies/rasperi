/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Controller
{
public:
    Controller();
    void showUi();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
