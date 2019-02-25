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

private slots:
    void on_actionImportModel_triggered();
    void on_actionSaveImage_triggered();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
