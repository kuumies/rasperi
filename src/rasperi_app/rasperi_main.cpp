/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Rasperi application.
 * ---------------------------------------------------------------- */

#include <iostream>
#include <QDebug>
#include <QtWidgets/QApplication>
#include <glm/gtx/string_cast.hpp>
#include "rasperi_controller.h"
#include "rasperi_main_window.h"
#include "rasperi_ext/rasperi_triangle_clipper.h"

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
int main(int argc, char** argv)
{
    using namespace kuu;
    using namespace kuu::rasperi;

    QApplication app(argc, argv);

//    auto proj = glm::perspective(M_PI, 1.0, 0.1, 150.0);
//    auto view = glm::translate(glm::dmat4(1.0), glm::dvec3(0, 0, 5));
//    auto cam  = proj * glm::inverse(view);
//    auto vp   = glm::vec4(0, 0, 512, 512);

//    Triangle tri;
//    tri.p1.position = glm::dvec3(-1, -1, -100);
//    tri.p2.position = glm::dvec3( 1, -1, -100);
//    tri.p3.position = glm::dvec3( 1,  1, -100);

//    TriangleClipper triClipper(cam, vp);
//    auto res = triClipper.clip(tri);
//    if (res.empty())
//        qDebug() << "tri not in frustum";
//    else
//    {
//        qDebug() << res.size() << "tris";
//        for (auto tri : res)
//        {
//            std::cout << glm::to_string(tri.p1.position) << ", "
//                      << glm::to_string(tri.p2.position) << ", "
//                      << glm::to_string(tri.p3.position) << std::endl;
//        }
//    }

//    return 0;

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
