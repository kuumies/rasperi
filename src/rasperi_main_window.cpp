/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::wakusei::MainWindow class.
 * ---------------------------------------------------------------- */

#include "rasperi_main_window.h"
#include "ui_rasperi_main_window.h"
#include <QtWidgets/QDockWidget>

namespace kuu
{
namespace wakusei
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct MainWindow::Impl
{
    Impl(Controller* controller, MainWindow* self)
        : controller(controller)
        , self(self)
    {}

    void createCentralWidget()
    {}

    Ui::MainWindow ui;
    Controller* controller;
    MainWindow* self = nullptr;
    QDockWidget* sideDockWidget = nullptr;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
MainWindow::MainWindow(Controller* controller, QWidget* parent)
    : QMainWindow(parent)
    , impl(std::make_shared<Impl>(controller, this))
{
    impl->ui.setupUi(this);
    impl->createCentralWidget();
}

} // namespace wakusei
} // namespace kuu
