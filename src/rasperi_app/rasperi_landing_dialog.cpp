/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::LandingDialog class.
 * ---------------------------------------------------------------- */

#include "rasperi_landing_dialog.h"
#include "ui_rasperi_landing_dialog.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct LandingDialog::Impl
{
    Ui::LandingDialog ui;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
LandingDialog::LandingDialog(QWidget* parent)
    : QDialog(parent)
    , impl(std::make_shared<Impl>())
{
    impl->ui.setupUi(this);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void LandingDialog::on_commandLinkButtonViewPbrSpheres_clicked()
{
    accept();
    emit viewPbrSphere();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void LandingDialog::on_commandLinkButtonImportPhong_clicked()
{
    accept();
    emit importPhong();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void LandingDialog::on_commandLinkButtonImportPbr_clicked()
{
    accept();
    emit importPbr();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void LandingDialog::on_commandLinkButtonExit_clicked()
{
    accept();
    emit exit();
}

} // namespace rasperi
} // namespace kuu
