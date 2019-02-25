/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::MainWindow class.
 * ---------------------------------------------------------------- */

#include "rasperi_main_window.h"
#include "ui_rasperi_main_window.h"
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

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
void MainWindow::on_actionImportModel_triggered()
{
    const QString filepath =
        QFileDialog::getOpenFileName(this, "Select Model",
                                    QDir::currentPath(),
                                     "*.obj *.fbx");
    if (filepath.isEmpty())
        return;

    if (!impl->controller->importModel(filepath))
    {
        QMessageBox::critical(this, "Model Import Failed",
                              "Failed to import model from " + filepath);
    }
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

} // namespace rasperi
} // namespace kuu
