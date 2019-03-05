/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::PbrIblIrradiance class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_pbr_ibl_irradiance.h"
#include <functional>
#include <iostream>
#include <QtCore/QDir>
#include "rasperi_cube_camera.h"
#include "rasperi_texture_cube.h"
#include "rasperi_texture_cube_mapping.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class IrradianceCubeRasterizer
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    class BoundingBox
    {
    public:
        BoundingBox()
        {
            min.x =  std::numeric_limits<double>::max();
            min.y =  std::numeric_limits<double>::max();
            max.x = -std::numeric_limits<double>::max();
            max.y = -std::numeric_limits<double>::max();
        }

        void update(const glm::dvec2& p)
        {
            if (p.x < min.x) min.x = p.x;
            if (p.y < min.y) min.y = p.y;
            if (p.x > max.x) max.x = p.x;
            if (p.y > max.y) max.y = p.y;
        }

        glm::dvec2 min;
        glm::dvec2 max;
    };

    using Callback = std::function<void(glm::dvec3)>;

    int w;
    int h;
    Callback callback;
    CubeCamera cubeCamera;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    IrradianceCubeRasterizer(int w, int h, Callback callback)
        : w(w)
        , h(h)
        , callback(callback)
        , cubeCamera(w / double(h))
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run() const
    {
        // --------------------------------------------------------
        // Create NDC cube

        std::vector<glm::dvec3> vertexData =
        {
            { -1,-1,-1 },
            {  1, 1,-1 },
            {  1,-1,-1 },
            { -1, 1,-1 },
            { -1,-1, 1 },
            {  1,-1, 1 },
            {  1, 1, 1 },
            { -1, 1, 1 },
        };

        std::vector<unsigned> indexData
        {
            2,1,0, 3,0,1,
            6,5,4, 4,7,6,
            0,3,7, 7,4,0,
            1,2,6, 5,6,2,
            5,2,0, 0,4,5,
            1,6,3, 7,3,6,
        };

        // --------------------------------------------------------
        // Rasterize

        for (int face = 0; face < 6; ++face)
        {
            std::cout << "Process face " << face << std::endl;

            glm::dmat4 camera = cubeCamera.cameraMatrix(size_t(face));

            #pragma omp parallel for
            for (int i = 0; i < indexData.size(); i += 3)
            {
                glm::dvec3 v1 = vertexData[indexData[i+0]];
                glm::dvec3 v2 = vertexData[indexData[i+1]];
                glm::dvec3 v3 = vertexData[indexData[i+2]];

                glm::dvec3 proj1 = project(camera, v1);
                glm::dvec3 proj2 = project(camera, v2);
                glm::dvec3 proj3 = project(camera, v3);

                glm::dvec2 vp1 = viewportTransform(proj1);
                glm::dvec2 vp2 = viewportTransform(proj2);
                glm::dvec2 vp3 = viewportTransform(proj3);

                BoundingBox bb;
                bb.update(vp1);
                bb.update(vp2);
                bb.update(vp3);

                int xmin = std::max(0, std::min(w - 1, int(std::floor(bb.min.x))));
                int ymin = std::max(0, std::min(h - 1, int(std::floor(bb.min.y))));
                int xmax = std::max(0, std::min(w - 1, int(std::floor(bb.max.x))));
                int ymax = std::max(0, std::min(h - 1, int(std::floor(bb.max.y))));

                for (int y = ymin; y <= ymax; ++y)
                for (int x = xmin; x <= xmax; ++x)
                {
                    glm::dvec2 screen(x + 0.5, y + 0.5);

                    // Barycentric weights
                    double w1 = edgeFunction(vp2, vp3, screen);
                    double w2 = edgeFunction(vp3, vp1, screen);
                    double w3 = edgeFunction(vp1, vp2, screen);
                    if (w1 < 0.0 || w2 < 0.0 || w3 < 0.0)
                        continue;

                    // Top-left edge rule
                    glm::dvec2 edge1 = vp2 - vp3;
                    glm::dvec2 edge2 = vp3 - vp1;
                    glm::dvec2 edge3 = vp1 - vp2;
                    bool overlaps = true;
                    overlaps &= (w1 == 0.0 ? ((edge1.y == 0.0 && edge1.x < 0.0) ||  edge1.y < 0.0) : (w1 > 0.0));
                    overlaps &= (w2 == 0.0 ? ((edge2.y == 0.0 && edge2.x < 0.0) ||  edge2.y < 0.0) : (w2 > 0.0));
                    overlaps &= (w3 == 0.0 ? ((edge3.y == 0.0 && edge3.x < 0.0) ||  edge3.y < 0.0) : (w3 > 0.0));
                    if (!overlaps)
                        continue;

                    double area = edgeFunction(vp1, vp2, vp3);
                    w1 /= area;
                    w2 /= area;
                    w3 /= area;

                    double z  = 1.0 / (w1 * 1.0 / proj1.z +
                                       w2 * 1.0 / proj2.z +
                                       w3 * 1.0 / proj3.z);

                    glm::dvec3 p1 = v1 / proj1.z;
                    glm::dvec3 p2 = v2 / proj2.z;
                    glm::dvec3 p3 = v3 / proj3.z;
                    glm::dvec3 p = p1  * w1 * z +
                                   p2  * w2 * z +
                                   p3  * w3 * z;

                    callback(p);
                }
            }
        }
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec3 project(const glm::dmat4& m, const glm::dvec3& p) const
    {
        const glm::dvec4 v = m * glm::dvec4(p, 1.0);
        if (v.w == 0.0)
        {
            std::cerr << "proj err" << std::endl;
            return glm::dvec3(0.0);
        }
        return glm::dvec3(v.x / v.w, v.y / v.w, v.z / v.w);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec2 viewportTransform(const glm::dvec3& p) const
    {
        const glm::ivec2 vp(w - 1, h - 1);
        const glm::dvec2 halfViewport = glm::dvec2(vp) * 0.5;

        glm::dvec2 out;
        out.x =        (p.x + 1.0) * halfViewport.x;
        out.y = vp.y - (p.y + 1.0) * halfViewport.y;

        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    double edgeFunction(const glm::dvec2& a,
                        const glm::dvec2& b,
                        const glm::dvec2& c) const
    {
        return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
    }
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct PbrIblIrradiance::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    class BoundingBox
    {
    public:
        BoundingBox()
        {
            min.x =  std::numeric_limits<double>::max();
            min.y =  std::numeric_limits<double>::max();
            max.x = -std::numeric_limits<double>::max();
            max.y = -std::numeric_limits<double>::max();
        }

        void update(const glm::dvec2& p)
        {
            if (p.x < min.x) min.x = p.x;
            if (p.y < min.y) min.y = p.y;
            if (p.x > max.x) max.x = p.x;
            if (p.y > max.y) max.y = p.y;
        }

        glm::dvec2 min;
        glm::dvec2 max;
    };


    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(PbrIblIrradiance* self, int size)
        : self(self)
        , size(size)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(const QImage& bgMap)
    {
        // --------------------------------------------------------
        // Create SDR cubemap of background map

        const QImage bgMapScaled =
            bgMap.scaled(size, size,
                         Qt::KeepAspectRatioByExpanding);

        TextureCube<uchar, 4> bgCubeMap(size, size);
        for (size_t f = 0; f < 6; ++f)
        {
            QImage face(size, size, QImage::Format_RGB32);
            QPainter p(&face);
            p.drawImage(face.rect(), bgMapScaled, face.rect());

            memcpy(bgCubeMap.face(f).pixels().data(),
                   face.bits(),
                   size_t(face.sizeInBytes()));
        }

         // --------------------------------------------------------
        // Render irradiance map

        auto irradienceCallback = [&](const glm::dvec3& p)
        {
            glm::dvec3 normal= glm::normalize(p);
            glm::dvec3 up    = glm::dvec3(0.0, 1.0, 0.0);
            glm::dvec3 right = glm::cross(up, normal);
                       up    = glm::cross(normal, right);

            //double sampleDelta = 0.025;
            double sampleDelta = 0.1;

            // Sample hemisphere
            glm::dvec3 irradiance = glm::dvec3(0.0);
            double nrSamples = 0.0;
            for(double  phi = 0.0;     phi < 2.0 * M_PI; phi   += sampleDelta)
            for(double  theta = 0.0; theta < 0.5 * M_PI; theta += sampleDelta)
            {
                // spherical to cartesian (in tangent space)
                const glm::dvec3 tangentSample =
                    glm::dvec3(sin(theta) * cos(phi),
                               sin(theta) * sin(phi),
                               cos(theta));

                // tangent space to world
                const glm::dvec3 sampleVec =
                    tangentSample.x * right +
                    tangentSample.y * up +
                    tangentSample.z * normal;

                // Sample background
                const texture_cube_mapping::TextureCoordinate texCoord =
                    texture_cube_mapping::mapPoint(sampleVec);
                const std::array<uchar, 4> texColors =
                    bgCubeMap.face(size_t(texCoord.faceIndex)).pixel(texCoord.uv.x, texCoord.uv.y);
                glm::dvec3 texColor(texColors[0] / 255.0,
                                    texColors[1] / 255.0,
                                    texColors[2] / 255.0);

                // Amplify the sample to fake HDR
                texColor *= 2.0;

                irradiance += texColor * cos(theta) * sin(theta);
                nrSamples++;
            }
            irradiance = M_PI * irradiance * (1.0 / double(nrSamples));

            const texture_cube_mapping::TextureCoordinate texCoord =
                texture_cube_mapping::mapPoint(normal);

            glm::ivec2 sc = mapCoord(texCoord.uv);
            size_t face = size_t(texCoord.faceIndex);

            std::array<double, 4> pix = { irradiance.r, irradiance.g, irradiance.b, 1.0 };
            self->irradianceCubemap.face(face).setPixel(sc.x, sc.y, pix);
        };

        IrradianceCubeRasterizer rasterizer(size, size, irradienceCallback);
        rasterizer.run();

        self->irradianceCubemap.toQImage().save("/temp/mega.bmp");
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::ivec2 mapCoord(glm::dvec2 texCoord) const
    {
        texCoord.y = 1.0 - texCoord.y;
        int px = int(std::floor(texCoord.x * double(size - 1)));
        int py = int(std::floor(texCoord.y * double(size - 1)));
        return glm::ivec2(px, py);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool read(const QDir& dir)
    {
        return self->irradianceCubemap.read(
            dir.absoluteFilePath("pbr_ibl_irradiance.kuu"));
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool write(const QDir& dir)
    {
        return self->irradianceCubemap.write(
            dir.absoluteFilePath("pbr_ibl_irradiance.kuu"));
    }

    PbrIblIrradiance* self;
    int size;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PbrIblIrradiance::PbrIblIrradiance(int size)
    : irradianceCubemap(size, size)
    , impl(std::make_shared<Impl>(this, size))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIblIrradiance::read(const QDir& dir)
{ return impl->read(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIblIrradiance::write(const QDir& dir)
{ return impl->write(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void PbrIblIrradiance::run(const QImage& bgMap)
{ impl->run(bgMap); }

} // namespace rasperi
} // namespace kuu
