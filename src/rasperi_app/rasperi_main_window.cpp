/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::MainWindow class.
 * ---------------------------------------------------------------- */

#include "rasperi_main_window.h"
#include "ui_rasperi_main_window.h"
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSplitter>
#include "rasperi_lib/rasperi_model.h"
#include "rasperi_about_dialog.h"
#include "rasperi_landing_dialog.h"
#include "rasperi_import_pbr_models_dialog.h"
#include "rasperi_import_phong_models_dialog.h"
#include "rasperi_image_widget.h"
#include "rasperi_opengl_widget.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct MainWindow::Impl
{
    Impl(MainWindow *self, Controller* controller)
        : self(self)
        , controller(controller)
        , splitter(new QSplitter())
        , imageWidget(new ImageWidget(controller))
        , openglWidget(new OpenGLWidget(controller))
    {
        splitter->addWidget(imageWidget);
        splitter->addWidget(openglWidget);
    }

    void updateOpenGLWidgetVisibility()
    {
        const bool visible = ui.actionOpenGLReference->isChecked();
        if (visible)
            splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
        else
            splitter->setSizes(QList<int>({INT_MAX, 0}));
    }

    MainWindow *self;
    Ui::MainWindow ui;
    Controller* controller;
    QSplitter* splitter;
    ImageWidget* imageWidget;
    OpenGLWidget* openglWidget;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
MainWindow::MainWindow(Controller* controller, QWidget* parent)
    : QMainWindow(parent)
    , impl(std::make_shared<Impl>(this, controller))
{
    impl->ui.setupUi(this);
    impl->updateOpenGLWidgetVisibility();
    setWindowIcon(QIcon("://icons/resource_usage_protect.png"));
    setCentralWidget(impl->splitter);
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
ImageWidget& MainWindow::imageWidget()
{ return *impl->imageWidget; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLWidget& MainWindow::openglWidget()
{ return *impl->openglWidget; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void MainWindow::setReferenceEnabled(bool enabled)
{ impl->ui.actionOpenGLReference->setChecked(enabled); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool MainWindow::isReferenceEnabled() const
{ return impl->ui.actionOpenGLReference->isChecked(); }

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

    //dlg.load("C:/Users/Antti Jumpponen/Documents/build-rasperi-VS2017-Release with Debug Information/install/bin/models/phong/bsa_bantam/BSA_BantamD1_OBJ.obj");
    //impl->controller->importModels(dlg.models());

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
void MainWindow::showOpenGLReference(bool /*show*/)
{
    impl->updateOpenGLWidgetVisibility();
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
void MainWindow::on_actionOpenGLReference_toggled(bool show)
{
    showOpenGLReference(show);
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
