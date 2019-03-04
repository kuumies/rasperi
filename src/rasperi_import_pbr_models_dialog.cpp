/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::ImportPbrModelsDialog class.
 * ---------------------------------------------------------------- */

#include "rasperi_import_pbr_models_dialog.h"
#include "ui_rasperi_import_pbr_models_dialog.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct ImportPbrModelsDialog::Impl
{
    Ui::ImportPbrModelsDialog ui;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ImportPbrModelsDialog::ImportPbrModelsDialog(QWidget* parent)
    : QDialog(parent)
    , impl(std::make_shared<Impl>())
{
    impl->ui.setupUi(this);
}

} // namespace rasperi
} // namespace kuu
