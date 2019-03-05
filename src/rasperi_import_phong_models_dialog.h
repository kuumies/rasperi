/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::ImportPhongModelsDialog class.
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>
#include <QtWidgets/QDialog>

namespace kuu
{
namespace rasperi
{

class Model;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class ImportPhongModelsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportPhongModelsDialog(QWidget* parent = nullptr);
    std::vector<Model> models() const;

private slots:
    void on_pushButtonBrowse_clicked();
    void on_comboBoxModels_currentIndexChanged(int index);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
