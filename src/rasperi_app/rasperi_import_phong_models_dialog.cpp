/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::ImportPhongModelsDialog class.
 * ---------------------------------------------------------------- */

#include "rasperi_import_phong_models_dialog.h"
#include "ui_rasperi_import_phong_models_dialog.h"
#include <future>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include "rasperi_lib/rasperi_model_importer.h"
#include "rasperi_lib/rasperi_model.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct ImportPhongModelsDialog::Impl
{   
    Ui::ImportPhongModelsDialog ui;
    QDir dir;
    std::vector<Model> models;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ImportPhongModelsDialog::ImportPhongModelsDialog(QWidget* parent)
    : QDialog(parent)
    , impl(std::make_shared<Impl>())
{
    impl->ui.setupUi(this);
    impl->dir = QDir::current().absoluteFilePath("models/phong");
    impl->ui.buttonBox->button(QDialogButtonBox::Ok)->setText("Import All");
    impl->ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
std::vector<Model> ImportPhongModelsDialog::models() const
{ return impl->models; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPhongModelsDialog::on_pushButtonBrowse_clicked()
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
    dlg.setCancelButton(nullptr);
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
        model.material->model = Material::Model::Phong;

    impl->dir = QFileInfo(filepath).absoluteDir();

    impl->ui.labelAmbient->setEnabled(true);
    impl->ui.labelAmbientText->setEnabled(true);
    impl->ui.labelDiffuse->setEnabled(true);
    impl->ui.labelDiffuseText->setEnabled(true);
    impl->ui.labelSpecular->setEnabled(true);
    impl->ui.labelSpecularText->setEnabled(true);
    impl->ui.labelSpecularPower->setEnabled(true);
    impl->ui.labelSpecularPowerText->setEnabled(true);
    impl->ui.labelNormal->setEnabled(true);
    impl->ui.labelNormalText->setEnabled(true);

    impl->ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    impl->ui.lineEditName->setEnabled(true);
    impl->ui.lineEditName->setText(QFileInfo(filepath).fileName());
    impl->ui.comboBoxModels->blockSignals(true);
    impl->ui.comboBoxModels->setEnabled(true);
    impl->ui.comboBoxModels->clear();
    for (const auto& model : impl->models)
        impl->ui.comboBoxModels->addItem(QString::fromStdString(model.name));
    impl->ui.comboBoxModels->setCurrentIndex(0);
    impl->ui.comboBoxModels->blockSignals(false);
    on_comboBoxModels_currentIndexChanged(0);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPhongModelsDialog::on_comboBoxModels_currentIndexChanged(int index)
{
    const Model& model = impl->models[size_t(index)];

    QImage ambient = model.material->phong.ambientSampler.map();
    if (ambient.isNull())
    {
        ambient = QImage(16, 16, QImage::Format_RGB32);
        ambient.fill(QColor::fromRgbF(model.material->phong.ambient.r,
                                      model.material->phong.ambient.g,
                                      model.material->phong.ambient.b));
    }
    impl->ui.labelAmbient->setPixmap(QPixmap::fromImage(ambient.scaled(impl->ui.labelAmbient->size())));

    QImage diffuse = model.material->phong.diffuseSampler.map();
    if (diffuse.isNull())
    {
        diffuse = QImage(16, 16, QImage::Format_RGB32);
        diffuse.fill(QColor::fromRgbF(model.material->phong.diffuse.r,
                                      model.material->phong.diffuse.g,
                                      model.material->phong.diffuse.b));
    }
    impl->ui.labelDiffuse->setPixmap(QPixmap::fromImage(diffuse.scaled(impl->ui.labelDiffuse->size())));

    QImage specular = model.material->phong.specularSampler.map();
    if (specular.isNull())
    {
        specular = QImage(16, 16, QImage::Format_RGB32);
        specular.fill(QColor::fromRgbF(model.material->phong.specular.r,
                                       model.material->phong.specular.g,
                                       model.material->phong.specular.b));
    }
    impl->ui.labelSpecular->setPixmap(QPixmap::fromImage(specular.scaled(impl->ui.labelSpecular->size())));

    QImage specularPower = model.material->phong.specularSampler.map();
    if (specularPower.isNull())
    {
        int sp = qRound(128.0 / model.material->phong.specularPower * 255.0);
        specularPower = QImage(16, 16, QImage::Format_RGB32);
        specularPower.fill(QColor::fromRgb(sp, sp, sp));
    }
    impl->ui.labelSpecularPower->setPixmap(QPixmap::fromImage(specularPower.scaled(impl->ui.labelSpecularPower->size())));

    QImage normal = model.material->normalSampler.map();
    if (normal.isNull())
    {
        normal = QImage(16, 16, QImage::Format_RGB32);
        normal.fill(0);
    }
    impl->ui.labelNormal->setPixmap(QPixmap::fromImage(normal.scaled(impl->ui.labelNormal->size())));
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPhongModelsDialog::on_buttonBox_accepted()
{
    accept();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void ImportPhongModelsDialog::on_buttonBox_rejected()
{
    reject();
}

} // namespace rasperi
} // namespace kuu
