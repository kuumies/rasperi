/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::MainWindow class.
 * ---------------------------------------------------------------- */
#pragma once

#include <QtWidgets/QMainWindow>
#include "rasperi_controller.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        Controller* controller,
        QWidget* parent = nullptr);

    void showLandingDialog();

private slots:
    void viewPbrSphereScene();
    void showImportPhongModelsDialog();
    void showImportPbrModelsDialog();

    void on_actionViewPBRSphere_triggered();
    void on_actionImportModelsPhong_triggered();
    void on_actionImportModelsPbr_triggered();
    void on_actionSaveImage_triggered();

    void on_actionAbout_triggered();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
