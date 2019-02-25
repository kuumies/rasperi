/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#include "rasperi_controller.h"
#include <QtCore/QDebug>
#include "rasperi_image_widget.h"
#include "rasperi_main_window.h"
#include "rasperi_model_importer.h"
#include "rasperi_rasterizer.h"

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

        //albedoMap     = "/temp/my_msp.png";
//        smoothNormals = false;
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
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void renderDefaultScene(int w, int h)
    {
        Vertex v1, v2;
        v1.position.x = -4.0;
        v1.color.r = 1.0;
        v1.color.a = 1.0;
        v2.position.x =  4.0;
        v2.color.b = 1.0;
        v2.color.a = 1.0;

        Mesh line;
        line.vertices.push_back(v1);
        line.vertices.push_back(v2);
        line.indices.push_back(0);
        line.indices.push_back(1);

        Sphere sphere(1.0, 64, 64);
        Quad q;

        Rasterizer r(w, h);
        r.clear();
        r.setNormalMode(Rasterizer::NormalMode::Coarse);
        r.drawTriangleMesh(&sphere);
        r.drawLineMesh(&line);
        r.drawTriangleMesh(&q);

        ColorFramebuffer colorFramebuffer = r.colorFramebuffer();
        image = colorFramebuffer.toQImage();
    }

    void renderModels(int w, int h,
                      std::vector<ModelImporter::Model>& models)
    {
        BoundingBox bb;
        for (const ModelImporter::Model& model : models)
            for (const Vertex& v : model.mesh->vertices)
                bb.update(v.position);

        //qDebug() << bb.min.z << bb.max.z;
        double zDepth = 0.0;
        zDepth = std::max(zDepth, std::abs(bb.min.z) + std::abs(bb.max.z));
        zDepth = std::max(zDepth, std::abs(bb.min.y) + std::abs(bb.max.y));
        zDepth = std::max(zDepth, std::abs(bb.min.x) + std::abs(bb.max.x));
        glm::dvec3 center = (bb.max - bb.min) * 0.5;

        glm::dmat4 view = glm::translate(glm::dmat4(1.0), glm::dvec3(0.0, center.y, zDepth * 1.4));
        glm::dmat4 proj = glm::perspective(M_PI * 0.25, w / double(h), 0.1, zDepth * 2);

        Rasterizer r(w, h);
        r.clear();
        r.setViewMatrix(view);
        r.setProjectionMatrix(proj);
        r.setNormalMode(Rasterizer::NormalMode::Coarse);
        for (ModelImporter::Model& model : models)
        {
            r.setMaterial(*model.material);
            r.drawTriangleMesh(model.mesh.get());
        }

        ColorFramebuffer colorFramebuffer = r.colorFramebuffer();
        image = colorFramebuffer.toQImage();
    }

    Controller* self = nullptr;
    QImage image;
    MainWindow mainWindow;
    ImageWidget imageWidget;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Controller::Controller()
    : impl(std::make_shared<Impl>(this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Controller::showUi()
{
    impl->mainWindow.setWindowTitle("Rasperi");
    impl->renderDefaultScene(720, 576);
    impl->imageWidget.setImage(impl->image);
    impl->mainWindow.setCentralWidget(&impl->imageWidget);
    impl->mainWindow.showMaximized();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool Controller::importModel(const QString& filepath)
{
    std::vector<ModelImporter::Model> models =
        ModelImporter().import(filepath);
    if (models.empty())
        return false;

    impl->renderModels(impl->imageWidget.width(), impl->imageWidget.height(), models);
    impl->imageWidget.setImage(impl->image);

    return true;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool Controller::saveImage(const QString& filepath)
{ return impl->image.save(filepath); }

} // namespace rasperi
} // namespace kuu
