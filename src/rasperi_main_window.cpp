/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::MainWindow class.
 * ---------------------------------------------------------------- */

#include "rasperi_main_window.h"
#include "ui_rasperi_main_window.h"
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include "rasperi_about_dialog.h"
#include "rasperi_landing_dialog.h"
#include "rasperi_import_pbr_models_dialog.h"
#include "rasperi_import_phong_models_dialog.h"
#include "rasperi_model.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct MainWindow::Impl
{
    Impl(Controller* controller)
        : controller(controller)
    {}

    Ui::MainWindow ui;
    Controller* controller;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
MainWindow::MainWindow(Controller* controller, QWidget* parent)
    : QMainWindow(parent)
    , impl(std::make_shared<Impl>(controller))
{
    impl->ui.setupUi(this);
    setWindowIcon(QIcon("://icons/resource_usage_protect.png"));
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::showLandingDialog()
{
    LandingDialog dlg(this);

    connect(&dlg, &LandingDialog::viewPbrSphere,
            this, &MainWindow::viewPbrSphereScene);
    connect(&dlg, &LandingDialog::importPhong,
            this, &MainWindow::showImportPhongModelsDialog);
    connect(&dlg, &LandingDialog::importPbr,
            this, &MainWindow::showImportPbrModelsDialog);
    connect(&dlg, &LandingDialog::exit,
            this, &MainWindow::close);

    dlg.exec();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::viewPbrSphereScene()
{
    impl->controller->viewPbrSphereScene();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::showImportPhongModelsDialog()
{
    ImportPhongModelsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
        impl->controller->importModels(dlg.models());
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::showImportPbrModelsDialog()
{
    ImportPbrModelsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
        impl->controller->importModels(dlg.models());
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::on_actionViewPBRSphere_triggered()
{
    viewPbrSphereScene();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::on_actionImportModelsPhong_triggered()
{
    showImportPhongModelsDialog();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::on_actionImportModelsPbr_triggered()
{
    showImportPbrModelsDialog();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::on_actionSaveImage_triggered()
{
    const QString filepath =
        QFileDialog::getSaveFileName(this, "Select Image",
                                    QDir::currentPath(),
                                     "*.png");
    if (filepath.isEmpty())
        return;

    if (!impl->controller->saveImage(filepath))
    {
        QMessageBox::critical(this, "Image Save Failed",
                              "Failed to save image to " + filepath);
    }
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg(this);
    dlg.exec();
}

} // namespace rasperi
} // namespace kuu
