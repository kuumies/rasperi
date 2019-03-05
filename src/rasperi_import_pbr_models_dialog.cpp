/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::ImportPbrModelsDialog class.
 * ---------------------------------------------------------------- */

#include "rasperi_import_pbr_models_dialog.h"
#include "ui_rasperi_import_pbr_models_dialog.h"
#include <future>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include "rasperi_model_importer.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct ImportPbrModelsDialog::Impl
{
    Ui::ImportPbrModelsDialog ui;
    QDir dir;
    std::vector<Model> models;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ImportPbrModelsDialog::ImportPbrModelsDialog(QWidget* parent)
    : QDialog(parent)
    , impl(std::make_shared<Impl>())
{
    impl->ui.setupUi(this);
    impl->dir = QDir::current().absoluteFilePath("models/pbr");
    impl->ui.buttonBox->button(QDialogButtonBox::Ok)->setText("Import All");
    impl->ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}
/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
std::vector<Model> ImportPbrModelsDialog::models() const
{ return impl->models; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_pushButtonBrowse_clicked()
{
    const QString filepath =
        QFileDialog::getOpenFileName(this, "Select Model",
                                     impl->dir.absolutePath(),
                                     "Models (*.obj *.fbx)");
    if (filepath.isEmpty())
        return;

    QProgressDialog dlg(this);
    dlg.setWindowTitle("Importing models");
    dlg.setLabelText("Importing models... please wait...");
    dlg.setMinimumWidth(400);
    dlg.setRange(0, 0);
    dlg.show();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    std::future<std::vector<Model>> loadFuture =
        std::async(std::launch::async,
                   [&](const QString& filepath)
    {
        return ModelImporter().import(filepath);
    }, filepath);

    auto status = loadFuture.wait_for(std::chrono::milliseconds(0));
    while (status != std::future_status::ready)
    {
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        status = loadFuture.wait_for(std::chrono::milliseconds(10));
    }

    try
    {
        impl->models = loadFuture.get();
    }
    catch(const std::exception& ex)
    {
        std::cerr << __FUNCTION__
                  << ": failed to load models "
                  << ex.what()
                  << std::endl;
    }

    dlg.hide();
    dlg.reset();

    if (impl->models.empty())
    {
        QMessageBox::critical(this, "File is not valid",
                              "Failed to load models from the file.");
        return;
    }

    for (Model& model : impl->models)
        model.material->model = Material::Model::Pbr;

    impl->dir = QFileInfo(filepath).absoluteDir();

    impl->ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    impl->ui.pushButtonBrowseAlbedo->setEnabled(true);
    impl->ui.pushButtonBrowseRoughness->setEnabled(true);
    impl->ui.pushButtonBrowseMetallic->setEnabled(true);
    impl->ui.pushButtonBrowseAo->setEnabled(true);
    impl->ui.pushButtonBrowseNormal->setEnabled(true);
    impl->ui.lineEditName->setEnabled(true);
    impl->ui.lineEditName->setText(QFileInfo(filepath).fileName());
    impl->ui.comboBoxModels->setEnabled(true);
    impl->ui.comboBoxModels->blockSignals(true);
    impl->ui.comboBoxModels->clear();
    for (const auto& model : impl->models)
        impl->ui.comboBoxModels->addItem(QString::fromStdString(model.name));
    impl->ui.comboBoxModels->setCurrentIndex(0);
    impl->ui.comboBoxModels->blockSignals(false);
    on_comboBoxModels_currentIndexChanged(0);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_comboBoxModels_currentIndexChanged(int /*index*/)
{
    updateImageLabels();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_pushButtonBrowseAlbedo_clicked()
{
    std::string name = impl->ui.comboBoxModels->currentText().toStdString();
    QImage img = browseImageMap();
    for (auto& model : impl->models)
        if (model.name == name)
            model.material->pbr.albedoSampler.setMap(img);
    updateImageLabels();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_pushButtonBrowseRoughness_clicked()
{
    std::string name = impl->ui.comboBoxModels->currentText().toStdString();
    QImage img = browseImageMap();
    for (auto& model : impl->models)
        if (model.name == name)
            model.material->pbr.roughnessSampler.setMap(img);
    updateImageLabels();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_pushButtonBrowseMetallic_clicked()
{
    std::string name = impl->ui.comboBoxModels->currentText().toStdString();
    QImage img = browseImageMap();
    for (auto& model : impl->models)
        if (model.name == name)
            model.material->pbr.metalnessSampler.setMap(img);
    updateImageLabels();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_pushButtonBrowseAo_clicked()
{
    std::string name = impl->ui.comboBoxModels->currentText().toStdString();
    QImage img = browseImageMap();
    for (auto& model : impl->models)
        if (model.name == name)
            model.material->pbr.aoSampler.setMap(img);
    updateImageLabels();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_pushButtonBrowseNormal_clicked()
{
    std::string name = impl->ui.comboBoxModels->currentText().toStdString();
    QImage img = browseImageMap();
    for (auto& model : impl->models)
        if (model.name == name)
            model.material->normalSampler.setMap(img);
    updateImageLabels();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_buttonBox_accepted()
{
    accept();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::on_buttonBox_rejected()
{
    reject();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
QImage ImportPbrModelsDialog::browseImageMap()
{
    const QString filepath =
        QFileDialog::getOpenFileName(this, "Select Image",
                                    impl->dir.absolutePath(),
                                    "Images (*.png *.bmp *.jpg *.tga)");
    if (filepath.isEmpty())
        return QImage();

    QImage img(filepath);
    if (img.isNull())
    {
        QMessageBox::critical(this, "Image is not valid",
                              "Failed to load image from the file.");
        return QImage();
    }

    impl->dir = QFileInfo(filepath).absoluteDir();

    return img;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPbrModelsDialog::updateImageLabels()
{
    const size_t index = size_t(impl->ui.comboBoxModels->currentIndex());
    const Model& model = impl->models[index];

    QImage albedo = model.material->pbr.albedoSampler.map();
    if (albedo.isNull())
    {
        albedo = QImage(16, 16, QImage::Format_RGB32);
        albedo.fill(QColor::fromRgbF(model.material->pbr.albedo.r,
                                     model.material->pbr.albedo.g,
                                     model.material->pbr.albedo.b));
    }
    impl->ui.labelAlbedo->setPixmap(QPixmap::fromImage(albedo.scaled(impl->ui.labelAlbedo->size())));

    QImage roughness = model.material->pbr.roughnessSampler.map();
    if (roughness.isNull())
    {
        roughness = QImage(16, 16, QImage::Format_RGB32);
        roughness.fill(QColor::fromRgbF(model.material->pbr.roughness,
                                        model.material->pbr.roughness,
                                        model.material->pbr.roughness));
    }
    impl->ui.labelRoughness->setPixmap(QPixmap::fromImage(roughness.scaled(impl->ui.labelRoughness->size())));

    QImage metalness = model.material->pbr.metalnessSampler.map();
    if (metalness.isNull())
    {
        metalness = QImage(16, 16, QImage::Format_RGB32);
        metalness.fill(QColor::fromRgbF(model.material->pbr.metalness,
                                        model.material->pbr.metalness,
                                        model.material->pbr.metalness));
    }
    impl->ui.labelMetallic->setPixmap(QPixmap::fromImage(metalness.scaled(impl->ui.labelMetallic->size())));

    QImage ao = model.material->pbr.aoSampler.map();
    if (ao.isNull())
    {
        ao = QImage(16, 16, QImage::Format_RGB32);
        ao.fill(QColor::fromRgbF(model.material->pbr.ao,
                                 model.material->pbr.ao,
                                 model.material->pbr.ao));
    }
    impl->ui.labelAo->setPixmap(QPixmap::fromImage(ao.scaled(impl->ui.labelAo->size())));

    QImage normal = model.material->normalSampler.map();
    if (normal.isNull())
    {
        normal = QImage(16, 16, QImage::Format_RGB32);
        normal.fill(0);
    }
        impl->ui.labelNormal->setPixmap(QPixmap::fromImage(normal.scaled(impl->ui.labelNormal->size())));
}

} // namespace rasperi
} // namespace kuu
