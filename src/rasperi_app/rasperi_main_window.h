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

class ImageWidget;
class OpenGLWidget;

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

    ImageWidget& imageWidget();
    OpenGLWidget& openglWidget();

    void setReferenceEnabled(bool enabled);
    bool isReferenceEnabled() const;

public slots:
    void viewPbrSphereScene();
    void showImportPhongModelsDialog();
    void showImportPbrModelsDialog();
    void showOpenGLReference(bool show);

    void on_actionViewPBRSphere_triggered();
    void on_actionImportModelsPhong_triggered();
    void on_actionImportModelsPbr_triggered();
    void on_actionSaveImage_triggered();

    void on_actionOpenGLReference_toggled(bool show);

    void on_actionAbout_triggered();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
