/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#pragma once

#include <vector>
#include <memory>
#include <QtCore/QString>

namespace kuu
{
namespace rasperi
{

class Camera;
class CameraController;
struct Model;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Controller
{
public:
    Controller();

    std::shared_ptr<Camera> camera() const;
    std::shared_ptr<CameraController> cameraController() const;

    void setImageSize(int w, int h);
    void rasterize(bool filled);
    void showUi();
    bool importModel(const QString& filepath);
    bool importModels(const std::vector<Model>& models);
    bool saveImage(const QString& filepath);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
