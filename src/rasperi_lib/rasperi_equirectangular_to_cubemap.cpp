/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::EquirectangularToCubemap class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_equirectangular_to_cubemap.h"
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
class CubeRasterizer
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
    CubeRasterizer(int w, int h, Callback callback)
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

            //#pragma omp parallel for
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
struct EquirectangularToCubemap::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(EquirectangularToCubemap* self, int size)
        : self(self)
        , size(size)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    TextureCube<double, 4> run(const Texture2D<double, 4>& e)
    {
        TextureCube<double, 4> out(size, size);

        auto callback = [&](const glm::dvec3& p)
        {
            glm::dvec3 n = glm::normalize(p);
            glm::dvec2 uv = sampleSphericalMap(n);
            std::array<double, 4> c = e.pixel(uv.x, uv.y);

            const texture_cube_mapping::TextureCoordinate texCoord =
                texture_cube_mapping::mapPoint(n);

            glm::ivec2 sc = mapCoord(texCoord.uv);
            size_t face = size_t(texCoord.faceIndex);
            if (texCoord.faceIndex < 0 || texCoord.faceIndex > 5)
                qDebug() << texCoord.faceIndex << sc.x << sc.y << texCoord.uv.x << texCoord.uv.y;
            out.face(face).setPixel(sc.x, sc.y, c);
        };

        CubeRasterizer rasterizer(size, size, callback);
        rasterizer.run();

        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec2 sampleSphericalMap(const glm::dvec3& v)
    {
        const glm::dvec2 invAtan = glm::dvec2(0.1591, 0.3183);
        glm::dvec2 uv = glm::dvec2(std::atan2(v.z, v.x), std::asin(v.y));
        uv *= invAtan;
        uv += 0.5;
        return uv;
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

    EquirectangularToCubemap* self;
    int size;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
EquirectangularToCubemap::EquirectangularToCubemap(int size)
    : impl(std::make_shared<Impl>(this, size))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
TextureCube<double, 4> EquirectangularToCubemap::run(const Texture2D<double, 4>& e)
{ return impl->run(e); }

} // namespace rasperi
} // namespace kuu
