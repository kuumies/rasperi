/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::PbrIblPrefilter class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_pbr_ibl_prefilter.h"
#include <array>
#include <functional>
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtGui/QPainter>
#include "rasperi_cube_camera.h"
#include "rasperi_framebuffer.h"
#include "rasperi_sampler.h"
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

    using Callback = std::function<void(glm::dvec3, double, int, int)>;

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
            for (size_t face = 0; face < 6; ++face)
            {
                std::cout << "Process face " << face << std::endl;

                double roughness = mipmap / double(mipmapCount - 1);
                glm::dmat4 camera = cubeCamera.cameraMatrix(face);

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

    //                #pragma omp parallel for
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

                        callback(p, roughness, mipmap, face);
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

            memcpy(bgCubeMap.face(f).pixels().data(), face.bits(), face.sizeInBytes());
        }
        bgCubeMap.toQImage().save("/temp/mmm.bmp");

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
        // Render prefilter map

        auto prefilterCallback = [&](
                const glm::dvec3& p,
                double roughness,
                int mipmap,
                int face)
        {
            Texture2D<double, 4>& tex =
                self->prefilterCubemap.face(face).mipmap(size_t(mipmap));

#if 0
            /* ---------------------------------------------------------------- *
             * ---------------------------------------------------------------- */
            float radicalInverse_VdC(uint bits)
            {
                bits = (bits << 16u) | (bits >> 16u);
                bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
                bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
                bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
                bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
                return float(bits) * 2.3283064365386963e-10; // / 0x100000000
            }

            /* ---------------------------------------------------------------- *
             * ---------------------------------------------------------------- */
            vec2 hammersley(uint i, uint n)
            {
                return vec2(float(i)/float(n), radicalInverse_VdC(i));
            }

            /* ---------------------------------------------------------------- *
             * ---------------------------------------------------------------- */
            vec3 importanceSampleGGX(vec2 xi, vec3 n, float roughness)
            {
                float a = roughness*roughness;

                float phi = 2.0 * PI * xi.x;
                float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
                float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

                // from spherical coordinates to cartesian coordinates
                vec3 h;
                h.x = cos(phi) * sinTheta;
                h.y = sin(phi) * sinTheta;
                h.z = cosTheta;

                // from tangent-space vector to world-space sample vector
                vec3 up        = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0)
                                                  : vec3(1.0, 0.0, 0.0);
                vec3 tangent   = normalize(cross(up, n));
                vec3 bitangent = cross(n, tangent);

                vec3 sampleVec = tangent * h.x + bitangent * h.y + n * h.z;
                return normalize(sampleVec);
            }

            /* ---------------------------------------------------------------- *
               Cook-Torrance specular BRDF (GGX) normal distribution function.
             * ---------------------------------------------------------------- */
            float brdfNormalDistributionGGX(float nDotH, float a)
            {
                float nDotH2 = nDotH * nDotH;
                float a2     = a * a;
                float q      = (nDotH2 * (a2 -1.0) + 1.0);
                return a2 / (PI * q * q);
            }

            vec3 n = normalize(texCoord);
            vec3 r = n;
            vec3 v = r;

            const uint SAMPLE_COUNT = 2048u * 1u;
            float totalWeight = 0.0;
            vec3 prefilteredColor = vec3(0.0);
            for(uint i = 0u; i < SAMPLE_COUNT; ++i)
            {
                vec2 xi = hammersley(i, SAMPLE_COUNT);
                vec3 h  = importanceSampleGGX(xi, n, roughness);
                vec3 l  = normalize(2.0 * dot(v, h) * h - v);

                float nDotL = max(dot(n, l), 0.0);
                if(nDotL > 0.0)
                {
                    float nDotH = max(dot(n, h), 0.0);
                    float hDotV = max(dot(h, v), 0.0);
                    float d   = brdfNormalDistributionGGX(nDotH, roughness);
                    float pdf = (d * nDotH / (4.0 * hDotV)) + 0.0001;

                    // resolution of source cubemap (per face)
                    float resolution = 512.0;
                    float saTexel  = 4.0 *
                            PI / (6.0 * resolution * resolution);
                    float saSample = 1.0 /
                            (float(SAMPLE_COUNT) * pdf + 0.0001);

                    float mipLevel = roughness == 0.0
                            ? 0.0
                            : 0.5 * log2(saSample / saTexel);

                    prefilteredColor +=
                        textureLod(skyboxMap, l, mipLevel).rgb * nDotL;
                    //prefilteredColor += texture(skyboxMap, l).rgb * nDotL;
                    totalWeight      += nDotL;
                }
            }
            prefilteredColor = prefilteredColor / totalWeight;

            outColor = vec4(prefilteredColor, 1.0);0
#endif
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
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    bool read(const QDir& dir)
    {
        bool ok = true;
        //ok &= self->prefilter.read(dir.absoluteFilePath("prefilter.dbl"));
        return ok;
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    bool write(const QDir& dir)
    {
        bool ok = true;
        //ok &= self->prefilter.write(dir.absoluteFilePath("prefilter.dbl"));
        return ok;
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
void PbrIblPrefilter::run(const QImage& bgMap)
{ impl->run(bgMap); }

} // namespace rasperi
} // namespace kuu
