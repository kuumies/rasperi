/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#include "rasperi_controller.h"
#include <future>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProgressDialog>
#include <QtCore/QDebug>
#include <QtCore/QTime>
#include "rasperi_lib/rasperi_camera.h"
#include "rasperi_lib/rasperi_model_importer.h"
#include "rasperi_lib/rasperi_model.h"
#include "rasperi_lib/rasperi_pbr_ibl_irradiance.h"
#include "rasperi_lib/rasperi_pbr_ibl_prefilter.h"
#include "rasperi_lib/rasperi_pbr_ibl_brdf_integration.h"
#include "rasperi_lib/rasperi_rasterizer.h"
#include "rasperi_opengl_reference_rasterizer/rasperi_opengl_reference_rasterizer.h"
#include "rasperi_camera_controller.h"
#include "rasperi_image_widget.h"
#include "rasperi_main_window.h"
#include "rasperi_opengl_widget.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Sphere : public Mesh
{
    Sphere(double radius, int ringCount, int sectorCount)
    {
        using namespace glm;

        const double pi      = M_PI;
        const double half_pi = pi * 0.5;

        const double ringStep   = 1.0 / double(ringCount   - 1);
        const double sectorStep = 1.0 / double(sectorCount - 1);

        for(int r = 0; r < ringCount;   ++r)
        for(int s = 0; s < sectorCount; ++s)
        {
            double y = sin(half_pi + pi * r * ringStep);
            double x = cos(2.0 * pi * s * sectorStep) * sin(pi * r * ringStep);
            double z = sin(2.0 * pi * s * sectorStep) * sin(pi * r * ringStep);

            dvec3 pos = dvec3(x, y, z) * radius;
            dvec2 texCoord = dvec2(s * sectorStep, r * ringStep);
            dvec3 normal = normalize(pos - dvec3(0.0));
            dvec4 color = dvec4(1.0);

            Vertex v;
            v.position = pos;
            v.texCoord = texCoord;
            v.normal   = normal;
            v.color    = color;
            vertices.push_back(v);
        }

        for(int r = 0; r < ringCount   - 1; r++)
        for(int s = 0; s < sectorCount - 1; s++)
        {
            unsigned ia = unsigned((r+0) * sectorCount + (s+0));
            unsigned ib = unsigned((r+0) * sectorCount + (s+1));
            unsigned ic = unsigned((r+1) * sectorCount + (s+1));
            unsigned id = unsigned((r+1) * sectorCount + (s+0));

            indices.push_back(id);
            indices.push_back(ia);
            indices.push_back(ib);

            indices.push_back(ib);
            indices.push_back(ic);
            indices.push_back(id);
        }

        generateTangents();
    }

    void generateTangents()
    {
        bool triangles = (indices.size() % 3) == 0;
        if (!triangles)
        {
            std::cerr << __FUNCTION__
                      << ": Mesh is not a triangle mesh."
                      << std::endl;
            return;
        }

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            Vertex& v1 = vertices[indices[i + 0]];
            Vertex& v2 = vertices[indices[i + 1]];
            Vertex& v3 = vertices[indices[i + 2]];

            glm::dvec3 edge1 = v2.position - v1.position;
            glm::dvec3 edge2 = v3.position - v1.position;
            glm::dvec2 dUV1 = v2.texCoord - v1.texCoord;
            glm::dvec2 dUV2 = v3.texCoord - v1.texCoord;

            double f = 1.0 / (dUV1.x * dUV2.y -
                              dUV2.x * dUV1.y);

            glm::dvec3 tangent;
            tangent.x = f * (dUV2.y * edge1.x - dUV1.y * edge2.x);
            tangent.y = f * (dUV2.y * edge1.y - dUV1.y * edge2.y);
            tangent.z = f * (dUV2.y * edge1.z - dUV1.y * edge2.z);
            tangent = glm::normalize(tangent);

            glm::dvec3 bitangent;
            bitangent.x = f * (-dUV2.x * edge1.x + dUV1.x * edge2.x);
            bitangent.y = f * (-dUV2.x * edge1.y + dUV1.x * edge2.y);
            bitangent.z = f * (-dUV2.x * edge1.z + dUV1.x * edge2.z);
            bitangent = glm::normalize(bitangent);

            v1.tangent = tangent;
            v2.tangent = tangent;
            v3.tangent = tangent;

            v1.bitangent = bitangent;
            v2.bitangent = bitangent;
            v3.bitangent = bitangent;
        }
    }
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Quad : public Mesh
{
    Quad()
    {
        using namespace glm;

        kuu::rasperi::Vertex v1;
        v1.position.x = -1;
        v1.position.y = -1;
        v1.color.r = 1.0;
        v1.normal.z = 1;

        kuu::rasperi::Vertex v2;
        v2.position.x =  1;
        v2.position.y = -1;
        v2.normal.z = 1;
        v2.color.g = 1.0;

        kuu::rasperi::Vertex v3;
        v3.position.x =  1;
        v3.position.y =  1;
        v3.normal.z = 1;
        v3.color.b = 1.0;

        kuu::rasperi::Vertex v4;
        v4.position.x = -1;
        v4.position.y =  1;
        v4.normal.z = 1;
        v4.color.r = 1.0;
        v4.color.g = 1.0;

        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);

        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);
    }
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Controller::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    class BoundingBox
    {
    public:
        BoundingBox()
        {
            min.x =  std::numeric_limits<int>::max();
            min.y =  std::numeric_limits<int>::max();
            min.z =  std::numeric_limits<int>::max();
            max.x = -std::numeric_limits<int>::max();
            max.y = -std::numeric_limits<int>::max();
            max.z = -std::numeric_limits<int>::max();
        }
        void update(const glm::dvec3& p)
        {
            if (p.x < min.x) min.x = p.x;
            if (p.y < min.y) min.y = p.y;
            if (p.z < min.z) min.z = p.z;
            if (p.x > max.x) max.x = p.x;
            if (p.y > max.y) max.y = p.y;
            if (p.z > max.z) max.z = p.z;
        }

        glm::dvec3 min;
        glm::dvec3 max;
    };

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(Controller* self)
        : self(self)
        , mainWindow(self)
        , camera(std::make_shared<Camera>())
        , cameraController(std::make_shared<CameraController>(self))
        , rasterizer(720, 576)
        , pbrIblIrradiance(512)
        , pbrIblPrefilter(512)
        , pbrIblBrdfIntegration(512)
    {}

#if 0
    /* ------------------------------------------------------------- *
     * ------------------------------------------------------------- */
    void loadPbrModels()
    {
        const QDir modelDir     = "C:/Users/Antti Jumpponen/Downloads/4w0tjxp6ojpc-old_lantern_pbr";
        const QDir textureDir   = modelDir.absoluteFilePath("textures");
        const QString modelPath = modelDir.absoluteFilePath("lantern_obj.obj");

        ModelImporter importer;
        models = importer.import(modelPath);

        // Model bounding box
        Impl::BoundingBox bb;
        for (const Model& model : models)
            for (const Vertex& v : model.mesh->vertices)
                bb.update(v.position);

        // Model center point to origo
        glm::dvec3 center = (bb.max + bb.min) * 0.5;
        glm::dvec3 toOrigo = center - glm::dvec3(0.0);
        for (const Model& model : models)
            model.transform->position = -toOrigo;

        // Fit camera to view the whole model
        glm::dvec3 size = bb.max - bb.min;
        double distance = fittingDistance(
                    glm::dvec2(size.x, size.y),
                    glm::dvec4(0.0, 0.0,
                               imageWidget.width(),
                               imageWidget.height()),
                    camera->fieldOfView);

        cameraController->setZoomAmount(size.z / 5.0);
        camera->viewDistance = distance;
        camera->farPlane = distance * 2;

        QString pathAlbedo    = textureDir.absoluteFilePath("lantern_Base_Color.jpg");
        QString pathRoughness = textureDir.absoluteFilePath("lantern_Roughness.jpg");
        QString pathMetalness = textureDir.absoluteFilePath("lantern_Metallic.jpg");
        QString pathAo        = textureDir.absoluteFilePath("lantern_Mixed_AO.jpg");
        QString pathNormal    = textureDir.absoluteFilePath("lantern_Normal_OpenGL.jpg");
        QString pathOpacity   = textureDir.absoluteFilePath("lantern_Opacity.jpg");

        std::shared_ptr<Material> material = std::make_shared<Material>();
        material->model = Material::Model::Pbr;
        material->pbr.albedoSampler.setMap(QImage(pathAlbedo));
        material->pbr.albedoSampler.setLinearizeGamma(true);
        material->pbr.roughnessSampler.setMap(QImage(pathRoughness).convertToFormat(QImage::Format_Grayscale8));
        material->pbr.metalnessSampler.setMap(QImage(pathMetalness).convertToFormat(QImage::Format_Grayscale8));
        material->pbr.aoSampler.setMap(QImage(pathAo).convertToFormat(QImage::Format_Grayscale8));
        material->pbr.irradiance = &pbrIblIrradiance.irradianceCubemap;
        material->pbr.prefilter  = &pbrIblPrefilter.prefilterCubemap;
        material->pbr.brdfIntegration  = &pbrIblBrdfIntegration.brdfIntegration2dMap;
        material->normalSampler.setMap(QImage(pathNormal));
        material->opacitySampler.setMap(QImage(pathOpacity).convertToFormat(QImage::Format_Grayscale8));

        for (auto& model : models)
            model.material = material;
    }
#endif

   /* ------------------------------------------------------------- *
    * ------------------------------------------------------------- */
    void rasterize(bool filled)
    {
        QTime timer;
        timer.start();

        rasterizer.clear();
        rasterizer.setNormalMode(Rasterizer::NormalMode::Smooth);
        rasterizer.setViewMatrix(camera->viewMatrix());
        rasterizer.setProjectionMatrix(camera->projectionMatrix());
        for (Model& model : models)
        {
            if (model.transform)
                rasterizer.setModelMatrix(model.transform->matrix());
            if (model.material)
                rasterizer.setMaterial(*model.material);
            if (filled)
                rasterizer.drawFilledTriangleMesh(model.mesh.get());
            else
                rasterizer.drawEdgeLineTriangleMesh(model.mesh.get());
        }

        Framebuffer& framebuffer = rasterizer.framebuffer();
        image = framebuffer.colorTex.toQImage();
        mainWindow.imageWidget().setImage(image);

        qDebug() << __FUNCTION__ << timer.elapsed() << "ms";

        if (mainWindow.isReferenceEnabled())
        {
            OpenGLReferenceRasterizer::Scene scene;
            scene.view       = camera->viewMatrix();
            scene.projection = camera->projectionMatrix();
            scene.models     = models;

            OpenGLWidget& openglWidget = mainWindow.openglWidget();
            openglWidget.setScene(scene);
            openglWidget.update();
        }
    }

    /* ------------------------------------------------------------- *
     * ------------------------------------------------------------- */
    double fittingDistance(
        const glm::dvec2& size,
        const glm::dvec4& viewport,
        const double fieldOfView)
    {
        // Screen aspect ratio
        double aspect = viewport.z / viewport.w;

        // Calculate max distance "radius" that must be visible
        double radius = std::max(size.x, size.y * aspect) / 2.0;
        //radius /= 2.0f;

        // Calculate fov
        double fov = 0.5 * glm::radians(fieldOfView);
        if (aspect < 1.0)
            // Fov in x is smaller
            fov = std::atan(aspect * std::tan(fov));

        // Return distance
        return radius / std::sin(fov);
    }

    /* ------------------------------------------------------------- *
     * ------------------------------------------------------------- */
    void createPbrIbl()
    {
        QDir dir("/temp/");

        if (!pbrIblIrradiance.read(dir))
        {
            pbrIblIrradiance.run(mainWindow.imageWidget().bgImage());
            pbrIblIrradiance.write(dir);
        }

        if (!pbrIblPrefilter.read(dir))
        {
            pbrIblPrefilter.run(mainWindow.imageWidget().bgImage());
            pbrIblPrefilter.write(dir);
        }

        if (!pbrIblBrdfIntegration.read(dir))
        {
            pbrIblBrdfIntegration.run();
            pbrIblBrdfIntegration.write(dir);
        }
    }

    Controller* self = nullptr;
    QImage image;
    MainWindow mainWindow;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<CameraController> cameraController;
    Rasterizer rasterizer;
    std::vector<Model> models;
    PbrIblIrradiance pbrIblIrradiance;
    PbrIblPrefilter pbrIblPrefilter;
    PbrIblBrdfIntegration pbrIblBrdfIntegration;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Controller::Controller()
    : impl(std::make_shared<Impl>(this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
std::shared_ptr<Camera> Controller::camera() const
{ return impl->camera; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
std::shared_ptr<CameraController> Controller::cameraController() const
{ return impl->cameraController; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
MainWindow& Controller::mainWindow() const
{ return impl->mainWindow; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Controller::setImageSize(int w, int h)
{
    impl->camera->aspectRatio = w / double(h);
    impl->rasterizer = Rasterizer(w, h);
    impl->rasterize(true);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Controller::rasterize(bool filled)
{ impl->rasterize(filled); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Controller::showUi()
{
    impl->mainWindow.showMaximized();
    QApplication::processEvents();
    impl->mainWindow.showLandingDialog();
    //impl->mainWindow.showImportPhongModelsDialog();
    //impl->mainWindow.setReferenceEnabled(true);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Controller::viewPbrSphereScene()
{
    const QDir rootDir = QDir::current().absoluteFilePath("pbr_textures");

    struct PbrSphere
    {
        QDir dir;
        QString albedo;
        QString roughness;
        QString metal;
        QString ao;
        QString normal;
        glm::dvec3 position;
    };
    std::array<PbrSphere, 6> spheres;
    spheres[0] =
    {
        rootDir.absoluteFilePath("bamboo-wood-semigloss"),
        "bamboo-wood-semigloss-albedo.png",
        "bamboo-wood-semigloss-roughness.png",
        "bamboo-wood-semigloss-metal.png",
        "bamboo-wood-semigloss-ao.png",
        "bamboo-wood-semigloss-normal.png",
        glm::dvec3(-1.0, 0.5, 0.0)
    };
    spheres[1] =
    {
        rootDir.absoluteFilePath("blocksrough"),
        "blocksrough_basecolor.png",
        "blocksrough_roughness.png",
        "blocksrough_metallic.png",
        "blocksrough_ambientocclusion.png",
        "blocksrough_normal.png",
        glm::dvec3(0.0, 0.5, 0.0)
    };
    spheres[2] =
    {
        rootDir.absoluteFilePath("rustediron1-alt2"),
        "rustediron2_basecolor.png",
        "rustediron2_roughness.png",
        "rustediron2_metallic.png",
        "",
        "rustediron2_normal.png",
        glm::dvec3(1.0, 0.5, 0.0)
    };
    spheres[3] =
    {
        rootDir.absoluteFilePath("cratered-rock-albedo.png"),
        "cratered-rock-albedo.png",
        "cratered-rock-roughness.png",
        "cratered-rock-metalness.png",
        "cratered-rock-ao.png",
        "cratered-rock-normal.png",
        glm::dvec3(1.0, -0.5, 0.0)
    };
    spheres[4] =
    {
        rootDir.absoluteFilePath("oakfloor_fb1"),
        "oakfloor_basecolor.png",
        "oakfloor_roughness.png",
        "",
        "oakfloor_AO.png",
        "oakfloor_normal.png",
        glm::dvec3(0.0, -0.5, 0.0)
    };
    spheres[5] =
    {
        rootDir.absoluteFilePath("rustediron-streaks"),
        "rustediron-streaks_basecolor.png",
        "rustediron-streaks_roughness.png",
        "rustediron-streaks_metallic.png",
        "",
        "rustediron-streaks_normal.png",
        glm::dvec3(-1.0, -0.5, 0.0)
    };

    QProgressDialog dlg(&impl->mainWindow);
    dlg.setWindowTitle("Loading textures");
    dlg.setLabelText("Loading textures... please wait...");
    dlg.setMinimumWidth(400);
    dlg.setRange(0, 0);
    dlg.setCancelButton(nullptr);
    dlg.show();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    std::future<std::map<QString, QImage>> loadFuture =
        std::async(std::launch::async,
                   [&](const std::array<PbrSphere, 6>& spheres)
    {
        std::map<QString, QImage> out;
        for (const PbrSphere& sphere : spheres)
        {
            QImage albedo(sphere.dir.absoluteFilePath(sphere.albedo));
            if (albedo.format() != QImage::Format_RGB32)
                albedo = albedo.convertToFormat(QImage::Format_RGB32);
            out[sphere.albedo] = albedo.rgbSwapped();

            QImage roughness(sphere.dir.absoluteFilePath(sphere.roughness));
            if (roughness.format() != QImage::Format_Grayscale8)
                roughness = roughness.convertToFormat(QImage::Format_Grayscale8);
            out[sphere.roughness] = roughness;

            QImage metalness(sphere.dir.absoluteFilePath(sphere.metal));
            if (metalness.format() != QImage::Format_Grayscale8)
                metalness = metalness.convertToFormat(QImage::Format_Grayscale8);
            out[sphere.metal] = metalness;

            QImage ao(sphere.dir.absoluteFilePath(sphere.ao));
            if (ao.format() != QImage::Format_Grayscale8)
                ao = metalness.convertToFormat(QImage::Format_Grayscale8);
            out[sphere.ao] = ao;

            QImage normal(sphere.dir.absoluteFilePath(sphere.normal));
            if (normal.format() != QImage::Format_RGB32)
                normal = normal.convertToFormat(QImage::Format_RGB32);
            out[sphere.normal] = normal;
        }
        return out;
    },  spheres);

    auto status = loadFuture.wait_for(std::chrono::milliseconds(0));
    while (status != std::future_status::ready)
    {
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        status = loadFuture.wait_for(std::chrono::milliseconds(10));
    }

    std::map<QString, QImage> images;
    try
    {
        images = loadFuture.get();
    }
    catch(const std::exception& ex)
    {
        std::cerr << __FUNCTION__
                  << ": failed to load models "
                  << ex.what()
                  << std::endl;
    }

    dlg.hide();
    dlg.reset();

    std::vector<Model> models;
    for (const PbrSphere& sphere : spheres)
    {
        Model m;
        m.mesh = std::make_shared<Sphere>(0.5, 32, 16);
        m.material = std::make_shared<Material>();
        m.material->model = Material::Model::Pbr;
        m.material->pbr.albedoSampler.setMap(images[sphere.albedo]);
        m.material->pbr.roughnessSampler.setMap(images[sphere.roughness]);
        m.material->pbr.metalnessSampler.setMap(images[sphere.metal]);
        m.material->pbr.aoSampler.setMap(images[sphere.ao]);
        m.material->normalSampler.setMap(images[sphere.normal]);
        m.transform = std::make_shared<Transform>();
        m.transform->position = sphere.position;
        models.push_back(m);
    }

    importModels(models, false);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool Controller::importModel(const QString& filepath)
{
    std::vector<Model> models =
        ModelImporter().import(filepath);
    if (models.empty())
        return false;

    return importModels(models);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool Controller::importModels(const std::vector<Model>& models,
                              bool moveRelatedToOrigo)
{
    // Model bounding box
    Impl::BoundingBox bb;
    for (const Model& model : models)
        for (const Vertex& v : model.mesh->vertices)
            bb.update(v.position);

    std::vector<Model> pbrModels;

    for (const Model& model : models)
    {
        if (model.material->model != Material::Model::Pbr)
            continue;
        pbrModels.push_back(model);
    }

    if (!pbrModels.empty())
        impl->createPbrIbl();

    for (const Model& model : pbrModels)
    {
        model.material->pbr.irradiance      = &impl->pbrIblIrradiance.irradianceCubemap;
        model.material->pbr.prefilter       = &impl->pbrIblPrefilter.prefilterCubemap;
        model.material->pbr.brdfIntegration = &impl->pbrIblBrdfIntegration.brdfIntegration2dMap;
    }

    // Model center point to origo
    if (moveRelatedToOrigo)
    {
        glm::dvec3 center = (bb.max + bb.min) * 0.5;
        glm::dvec3 toOrigo = center - glm::dvec3(0.0);
        for (const Model& model : models)
            model.transform->position = -toOrigo;
    }

    // Fit camera to view the whole model
    glm::dvec3 size = bb.max - bb.min;
    double distance = impl->fittingDistance(
                glm::dvec2(size.x, size.y),
                glm::dvec4(0.0, 0.0,
                           impl->mainWindow.imageWidget().width(),
                           impl->mainWindow.imageWidget().height()),
                impl->camera->fieldOfView);

//    toOrigo.z = 100.0;
//    std::cout << __FUNCTION__          << ": "
//              << glm::to_string(bb.min)  << ", "
//              << glm::to_string(bb.max)  << ", "
//              << glm::to_string(center)
//              << std::endl;

    impl->cameraController->setZoomAmount(size.z / 5.0);
    impl->camera->viewDistance = distance;
    impl->camera->farPlane = distance * 2;

    impl->models = models;
    impl->rasterize(true);
    return true;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool Controller::saveImage(const QString& filepath)
{ return impl->image.save(filepath); }

} // namespace rasperi
} // namespace kuu
