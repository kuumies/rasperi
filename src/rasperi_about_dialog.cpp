/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::AboutDialog class.
 * ---------------------------------------------------------------- */

#include "rasperi_about_dialog.h"
#include "ui_rasperi_about_dialog.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct AboutDialog::Impl
{
    Ui::AboutDialog ui;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , impl(std::make_shared<Impl>())
{
    impl->ui.setupUi(this);
}

} // namespace rasperi
} // namespace kuu
