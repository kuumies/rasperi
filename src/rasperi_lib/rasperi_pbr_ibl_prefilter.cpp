/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::PbrIblPrefilter class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_pbr_ibl_prefilter.h"
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
class PrefilterCubeRasterizer
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

    using Callback = std::function<void(glm::dvec3, double, int)>;

    int w;
    int h;
    int mipmapCount;
    Callback callback;
    CubeCamera cubeCamera;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    PrefilterCubeRasterizer(int w, int h, int mipmapCount, Callback callback)
        : w(w)
        , h(h)
        , mipmapCount(mipmapCount)
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

        for (int mipmap = 0; mipmap < mipmapCount; ++mipmap)
        {
            std::cout << "Processing mipmap " << mipmap << std::endl;

            #pragma omp parallel for
            for (int face = 0; face < 6; ++face)
            {
                std::cout << "Process face " << face << std::endl;

                double roughness = mipmap / double(mipmapCount - 1);
                glm::dmat4 camera = cubeCamera.cameraMatrix(size_t(face));

                for (size_t i = 0; i < indexData.size(); i += 3)
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

                        callback(p, roughness, mipmap);
                    }
                }
            }
        }
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
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

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
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
struct PbrIblPrefilter::Impl
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
    Impl(PbrIblPrefilter* self, int size)
        : self(self)
        , size(size)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(const TextureCube<double, 4>& bgCubeMap)
    {
        // --------------------------------------------------------
        // Render prefilter map

        auto prefilterCallback = [&](
                const glm::dvec3& p,
                double roughness,
                int mipmap)
        {
            glm::dvec3 n = normalize(p);
            glm::dvec3 r = n;
            glm::dvec3 v = r;

            const uint SAMPLE_COUNT = 1024u;
            double totalWeight = 0.0;
            glm::dvec3 prefilteredColor = glm::dvec3(0.0);
            for(uint i = 0u; i < SAMPLE_COUNT; ++i)
            {
                glm::dvec2 xi = hammersley(i, SAMPLE_COUNT);
                glm::dvec3 h  = importanceSampleGGX(xi, n, roughness);
                glm::dvec3 l  = normalize(2.0 * dot(v, h) * h - v);

                double nDotL = glm::max(glm::dot(n, l), 0.0);
                if(nDotL > 0.0)
                {
                    // Sample background
                    const texture_cube_mapping::TextureCoordinate tc =
                        texture_cube_mapping::mapPoint(l);
                    const size_t faceIndex = size_t(tc.faceIndex);
                    const std::array<double, 4> pixel =
                        bgCubeMap.face(faceIndex).pixel(tc.uv.x, tc.uv.y);
                    prefilteredColor += glm::dvec3(
                            pixel[0],
                            pixel[1],
                            pixel[2]) * nDotL;
                    totalWeight += nDotL;
                }
            }
            prefilteredColor = prefilteredColor / totalWeight;
            std::array<double, 4> pix = { prefilteredColor.r,
                                          prefilteredColor.g,
                                          prefilteredColor.b,
                                          1.0 };

            const texture_cube_mapping::TextureCoordinate tc =
                texture_cube_mapping::mapPoint(n);
            const size_t faceIndex = size_t(tc.faceIndex);

            Texture2D<double, 4>& tex =
                self->prefilterCubemap.face(size_t(faceIndex)).mipmap(size_t(mipmap));
            tex.setPixel(tc.uv.x, tc.uv.y, pix);
        };

        if (!self->prefilterCubemap.generateMipmaps())
            std::cerr << __FUNCTION__
                      << ": failed to generate mipmaps"
                      << std::endl;

        const int mipmapCount = self->prefilterCubemap.mipmapCount();
        std::cout << __FUNCTION__ << ": " << mipmapCount << std::endl;

        PrefilterCubeRasterizer rasterizer(size, size,
                                           mipmapCount,
                                           prefilterCallback);
        rasterizer.run();

        for (int i = 0; i < mipmapCount; ++i)
            self->prefilterCubemap.toQImage(i).save(QString("/temp/00_prefilter_%1.bmp").arg(i));
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    double radicalInverse_VdC(uint bits)
    {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return double(bits) * 2.3283064365386963e-10; // / 0x100000000
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec2 hammersley(uint i, uint n)
    {
        return glm::dvec2(double(i) / double(n), radicalInverse_VdC(i));
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec3 importanceSampleGGX(glm::dvec2 xi, glm::dvec3 n, double roughness)
    {
        double a = roughness * roughness;

        double phi = 2.0 * M_PI * xi.x;
        double cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
        double sinTheta = sqrt(1.0 - cosTheta*cosTheta);

        // from spherical coordinates to cartesian coordinates
        glm::dvec3 h;
        h.x = cos(phi) * sinTheta;
        h.y = sin(phi) * sinTheta;
        h.z = cosTheta;

        // from tangent-space vector to world-space sample vector
        glm::dvec3 up = glm::abs(n.z) < 0.999
                ? glm::dvec3(0.0, 0.0, 1.0)
                : glm::dvec3(1.0, 0.0, 0.0);
        glm::dvec3 tangent   = glm::normalize(glm::cross(up, n));
        glm::dvec3 bitangent = glm::cross(n, tangent);

        glm::dvec3 sampleVec = tangent * h.x + bitangent * h.y + n * h.z;
        return normalize(sampleVec);
    }

    /* ---------------------------------------------------------------- *
       Cook-Torrance specular BRDF (GGX) normal distribution function.
     * ---------------------------------------------------------------- */
    double brdfNormalDistributionGGX(double nDotH, double a)
    {
        double nDotH2 = nDotH * nDotH;
        double a2     = a * a;
        double q      = (nDotH2 * (a2 -1.0) + 1.0);
        return a2 / (M_PI * q * q);
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    bool read(const QDir& dir)
    {
        return self->prefilterCubemap.read(dir.absoluteFilePath("pbr_ibl_prefilter.kuu"));
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    bool write(const QDir& dir)
    {
        return self->prefilterCubemap.write(dir.absoluteFilePath("pbr_ibl_prefilter.kuu"));
    }

    PbrIblPrefilter* self;
    int size;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PbrIblPrefilter::PbrIblPrefilter(int size)
    : prefilterCubemap(size, size)
    , impl(std::make_shared<Impl>(this, size))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIblPrefilter::read(const QDir& dir)
{ return impl->read(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIblPrefilter::write(const QDir& dir)
{ return impl->write(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void PbrIblPrefilter::run(const TextureCube<double, 4>& bgCubeMap)
{ impl->run(bgCubeMap); }

} // namespace rasperi
} // namespace kuu
