/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::ImportPbrModelsDialog class.
 * ---------------------------------------------------------------- */

#pragma once

#include <vector>
#include <memory>
#include <QtWidgets/QDialog>

namespace kuu
{
namespace rasperi
{

class Model;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class ImportPbrModelsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportPbrModelsDialog(QWidget* parent = nullptr);
    std::vector<Model> models() const;

private slots:
    void on_pushButtonBrowse_clicked();
    void on_comboBoxModels_currentIndexChanged(int index);
    void on_pushButtonBrowseAlbedo_clicked();
    void on_pushButtonBrowseRoughness_clicked();
    void on_pushButtonBrowseMetallic_clicked();
    void on_pushButtonBrowseAo_clicked();
    void on_pushButtonBrowseNormal_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    QImage browseImageMap();
    void updateImageLabels();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
