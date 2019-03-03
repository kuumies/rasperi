/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::PbrIblBrdfIntegration class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_pbr_ibl_brdf_integration.h"
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
class BrdfIntegrationRasterizer
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

    using Callback = std::function<void(glm::dvec2)>;

    int w;
    int h;
    Callback callback;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    BrdfIntegrationRasterizer(int w, int h, Callback callback)
        : w(w)
        , h(h)
        , callback(callback)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run() const
    {
        double xStep = 1.0 / double(w);
        double yStep = 1.0 / double(h);
        for (double y = 0.0; y <= 1.0; y += yStep)
        for (double x = 0.0; x <= 1.0; x += xStep)
                callback(glm::dvec2(x, y));
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
struct PbrIblBrdfIntegration::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(PbrIblBrdfIntegration* self, int size)
        : self(self)
        , size(size)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run()
    {
        // --------------------------------------------------------
        // Render BRDF integration map

        auto brdfIntegrationCallback = [&](const glm::dvec2& texCoord)
        {
            std::cout << __FUNCTION__  << ": " << glm::to_string(texCoord) <<  std::endl;
            glm::dvec2 integratedBRDF = integrateBRDF(texCoord.x, texCoord.y);

            std::array<double, 2> pix = { integratedBRDF.x, integratedBRDF.y };
            //std::array<double, 2> pix = { texCoord.x, texCoord.y };
            self->brdfIntegration2dMap.setPixel(texCoord.x, texCoord.y, pix);

//            Texture2D<double, 4>& tex =
//                self->prefilterCubemap.face(size_t(faceIndex)).mipmap(size_t(mipmap));
//            tex.setPixel(tc.uv.x, tc.uv.y, pix);


        };

        BrdfIntegrationRasterizer rasterizer(size, size,
                                             brdfIntegrationCallback);
        rasterizer.run();

        self->brdfIntegration2dMap.toQImage().save("/temp/00_brdg.bmp");
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec2 integrateBRDF(double nDotV, double roughness)
    {
        glm::dvec3 v;
        v.x = sqrt(1.0 - nDotV * nDotV);
        v.y = 0.0;
        v.z = nDotV;

        double a = 0.0;
        double b = 0.0;

        glm::dvec3 n = glm::dvec3(0.0, 0.0, 1.0);

        //const uint SAMPLE_COUNT = 10u;
        const uint SAMPLE_COUNT = 1024u;
        for(uint i = 0u; i < SAMPLE_COUNT; ++i)
        {
            glm::dvec2 xi = hammersley(i, SAMPLE_COUNT);
            glm::dvec3 h  = importanceSampleGGX(xi, n, roughness);
            glm::dvec3 l  = normalize(2.0 * dot(v, h) * h - v);

            double nDotL = glm::max(l.z, 0.0);
            double nDotH = glm::max(h.z, 0.0);
            double vDotH = glm::max(glm::dot(v, h), 0.0);

            if(nDotL > 0.0)
            {
                double g = geometrySmith(n, v, l, roughness);
                double gVis = (g * vDotH) / (nDotH * nDotV);
                double fc = pow(1.0 - vDotH, 5.0);

                a += (1.0 - fc) * gVis;
                b += fc * gVis;
            }
        }
        a /= double(SAMPLE_COUNT);
        b /= double(SAMPLE_COUNT);
        return glm::dvec2(a, b);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    double geometrySchlickGGX(double nDotV, double roughness)
    {
        double a = roughness;
        double k = (a * a) / 2.0;

        double nom   = nDotV;
        double denom = nDotV * (1.0 - k) + k;

        return nom / denom;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    double geometrySmith(glm::dvec3 N, glm::dvec3 V, glm::dvec3 L, double roughness)
    {
        double NdotV = glm::max(glm::dot(N, V), 0.0);
        double NdotL = glm::max(glm::dot(N, L), 0.0);
        double ggx2 = geometrySchlickGGX(NdotV, roughness);
        double ggx1 = geometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
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

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool read(const QDir& dir)
    {
        return self->brdfIntegration2dMap.read(
            dir.absoluteFilePath("pbr_ibl_brdf_integration.kuu"));
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    bool write(const QDir& dir)
    {
        return self->brdfIntegration2dMap.write(
            dir.absoluteFilePath("pbr_ibl_brdf_integration.kuu"));
    }

    PbrIblBrdfIntegration* self;
    int size;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PbrIblBrdfIntegration::PbrIblBrdfIntegration(int size)
    : brdfIntegration2dMap(size, size)
    , impl(std::make_shared<Impl>(this, size))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIblBrdfIntegration::read(const QDir& dir)
{ return impl->read(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIblBrdfIntegration::write(const QDir& dir)
{ return impl->write(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void PbrIblBrdfIntegration::run()
{ impl->run(); }

} // namespace rasperi
} // namespace kuu
