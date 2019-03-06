/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Rasperi application.
 * ---------------------------------------------------------------- */

#include <iostream>
#include <QtWidgets/QApplication>
#include "rasperi_controller.h"
#include "rasperi_main_window.h"

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
int main(int argc, char** argv)
{
    using namespace kuu;
    using namespace kuu::rasperi;

    QApplication app(argc, argv);

    try
    {
        Controller controller;
        controller.showUi();

        if (controller.mainWindow().isVisible())
            app.exec();
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
