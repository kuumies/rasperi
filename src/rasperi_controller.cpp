/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Implementation of kuu::rasperi::Controller class.
 * ---------------------------------------------------------------- */

#include "rasperi_controller.h"
#include "rasperi_image_widget.h"
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
    Impl(Controller* self)
        : self(self)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void render(int w, int h)
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

    Controller* self = nullptr;
    QImage image;
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
    impl->imageWidget.setWindowTitle("Rasperi");
    impl->imageWidget.resize(720, 576);
    impl->imageWidget.show();
    impl->render(720, 576);
    impl->imageWidget.setImage(impl->image);
}

} // namespace rasperi
} // namespace kuu
