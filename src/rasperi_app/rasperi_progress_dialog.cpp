/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#include "rasperi_progress_dialog.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct ProgressDialog::Impl
{};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ProgressDialog::ProgressDialog(QWidget* parent)
    : QProgressDialog(parent)
    , impl(std::make_shared<Impl>())
{
    connect(this, &ProgressDialog::doSetProgress,
            this, &ProgressDialog::onDoSetProgress);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ProgressDialog::setProgress(int min, int value, int max)
{
    emit doSetProgress(min, value, max);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ProgressDialog::onDoSetProgress(int min, int value, int max)
{
    setRange(min, max);
    setValue(value);
}

} // namespace rasperi
} // namespace kuu
