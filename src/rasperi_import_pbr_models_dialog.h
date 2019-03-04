/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::ImportPbrModelsDialog class.
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>
#include <QtWidgets/QDialog>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class ImportPbrModelsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportPbrModelsDialog(QWidget* parent = nullptr);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
