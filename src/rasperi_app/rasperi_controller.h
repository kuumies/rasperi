/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#pragma once

#include <vector>
#include <memory>

class QString;

namespace kuu
{
namespace rasperi
{

class Camera;
class CameraController;
class MainWindow;
class Model;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Controller
{
public:
    Controller();

    std::shared_ptr<Camera> camera() const;
    std::shared_ptr<CameraController> cameraController() const;
    MainWindow& mainWindow() const;

    void setImageSize(int w, int h);
    void rasterize(bool filled);
    void showUi();
    void viewPbrSphereScene();
    bool importModel(const QString& filepath);
    bool importModels(const std::vector<Model>& models,
                      bool moveRelatedToOrigo = true);
    bool saveImage(const QString& filepath);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
