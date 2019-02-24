/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::wakusei::rasterizer::Rasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_rasterizer.h"
#include <array>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtGui/QImage>

namespace kuu
{
namespace wakusei
{
namespace rasterizer
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Sampler
{
public:
    enum class Filter
    {
        Nearest,
        Linear
    };

    Sampler(const QImage& map,
            Filter filter = Filter::Nearest,
            bool linearizeGamma = false)
        : map(map)
        , filter(filter)
        , linearizeGamma(linearizeGamma)
    {
        if (map.isNull())
            throw std::runtime_error("Invalid texture map");
    }

    glm::dvec4 sampleRgba(const glm::dvec2& texCoord) const
    {
        switch(filter)
        {
            case Filter::Nearest: return sampleRgbaNearest(texCoord);
            case Filter::Linear:  return sampleRgbaLinear(texCoord);
        }
        return glm::dvec4(0.0);
    }

private:
    glm::dvec4 sampleRgbaNearest(const glm::dvec2& texCoord) const
    {
        int px = int(std::floor(texCoord.x * double(map.width()  - 1)));
        int py = int(std::floor(texCoord.y * double(map.height() - 1)));
        return sampleRgba(px, py);
    }

    glm::dvec4 sampleRgbaLinear(const glm::dvec2& texCoord) const
    {
        int px = int(std::floor(texCoord.x * double(map.width()  - 1)));
        int py = int(std::floor(texCoord.y * double(map.height() - 1)));

        std::array<glm::dvec4, 4> pixels;
        pixels[0] = sampleRgba(px,     py    );
        pixels[1] = sampleRgba(px + 1, py    );
        pixels[2] = sampleRgba(px,     py + 1);
        pixels[3] = sampleRgba(px + 1, py + 1);

        return bilinear<glm::dvec4>(texCoord.x, texCoord.y,
                                    pixels[0],
                                    pixels[1],
                                    pixels[2],
                                    pixels[3]);
    }

    glm::dvec4 sampleRgba(int x, int y) const
    {
        const QRgb* line = reinterpret_cast<const QRgb*>(map.scanLine(y));
        const QRgb pixel = line[x];

        glm::dvec4 out(
            double(qRed(pixel))   / 255.0,
            double(qGreen(pixel)) / 255.0,
            double(qBlue(pixel))  / 255.0,
            double(qAlpha(pixel)) / 255.0);
        out = glm::clamp(out, glm::dvec4(0.0), glm::dvec4(1.0));

        if (linearizeGamma)
        {
            double gamma = 2.2;
            out = glm::pow(out, glm::dvec4(gamma));
        }

        return out;
    }

    // https://www.scratchapixel.com/lessons/mathematics-physics-for-
    // computer-graphics/interpolation/bilinear-filtering
    template<typename T>
    T bilinear(
       const double tx,
       const double ty,
       const T& c00,
       const T& c10,
       const T& c01,
       const T& c11) const
    {
        T a = c00 * (1.0 - tx) + c10 * tx;
        T b = c01 * (1.0 - tx) + c11 * tx;
        return a * (1.0 - ty) + b * ty;
    }

private:
    QImage map;
    Filter filter;
    bool linearizeGamma;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class PrimitiveRasterizer
{
public:
    PrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                        DepthFramebuffer& depthbuffer)
        : colorbuffer(colorbuffer)
        , depthbuffer(depthbuffer)
    {}

    virtual ~PrimitiveRasterizer()
    {}

    // Projection
    glm::dvec3 project(const glm::dmat4& m,
                       const glm::dvec3& p)
    {
        const glm::dvec4 v = m * glm::dvec4(p, 1.0);
        if (v.w == 0.0)
            return glm::dvec3(0.0);
        return glm::dvec3(v.x / v.w, v.y / v.w, v.z / v.w);
    }

    // Viewport transform
    glm::ivec2 viewportTransform(const glm::dvec3& p)
    {
        const glm::ivec2 vp(colorbuffer.width, colorbuffer.height);
        const glm::dvec2 halfViewport = glm::dvec2(vp) * 0.5;

        glm::ivec2 out;
        out.x =        int(std::floor((p.x + 1.0) * halfViewport.x));
        out.y = vp.y - int(std::floor((p.y + 1.0) * halfViewport.y));

        glm::ivec2 vpMin = glm::ivec2(0, 0);
        glm::ivec2 vpMax = glm::ivec2(vp.x - 1, vp.y- 1);
        return glm::clamp(out, vpMin, vpMax);
    }

    // Set colorbuffer value
    void setRgba(int x, int y, glm::dvec4 c)
    {
        c.r = glm::clamp(c.r, 0.0, 1.0);
        c.g = glm::clamp(c.g, 0.0, 1.0);
        c.b = glm::clamp(c.b, 0.0, 1.0);
        c.a = glm::clamp(c.a, 0.0, 1.0);

        using uchar = unsigned char;
        unsigned char r = uchar(std::floor(c.r * 255.0));
        unsigned char g = uchar(std::floor(c.g * 255.0));
        unsigned char b = uchar(std::floor(c.b * 255.0));
        unsigned char a = uchar(std::floor(c.a * 255.0));

        size_t loc = size_t(y * colorbuffer.width * colorbuffer.channels +
                            x * colorbuffer.channels);

        ColorFramebuffer::Data& d = *colorbuffer.data.get();
        d[loc + 0] = r;
        d[loc + 1] = g;
        d[loc + 2] = b;
        d[loc + 3] = a;
    }

protected:
    ColorFramebuffer& colorbuffer;
    DepthFramebuffer& depthbuffer;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class LinePrimitiveRasterizer : public PrimitiveRasterizer
{
public:
    LinePrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                            DepthFramebuffer& depthbuffer)
        : PrimitiveRasterizer(colorbuffer, depthbuffer)
    {}

    void rasterize(const Mesh& m, const glm::dmat4& matrix)
    {
        for (size_t i = 0; i < m.indices.size(); i += 2)
        {
            unsigned i1 = m.indices[i + 0];
            unsigned i2 = m.indices[i + 1];
            Vertex v1 = m.vertices[i1];
            Vertex v2 = m.vertices[i2];

            rasterize(v1, v2, matrix);
        }
    }

    void rasterize(const Vertex& v1,
                   const Vertex& v2,
                   const glm::dmat4& matrix)
    {
        // Projection
        glm::dvec3 p1 = project(matrix, v1.position);
        glm::dvec3 p2 = project(matrix, v2.position);

        // Viewport transform
        glm::dvec2 vpP1 = viewportTransform(p1);
        glm::dvec2 vpP2 = viewportTransform(p2);
        glm::dvec2 diff = vpP2 - vpP1;
        glm::dvec2 dir = glm::normalize(vpP2 - vpP1);
        double a = glm::length(diff);
        double d = glm::length(dir);

        for (double r = 0.0; r <= a; r += d)
        {
            double t = r / a;
            glm::ivec2 p = vpP1 + dir * r;
            glm::dvec4 c = glm::mix(v1.color, v2.color, t);

            // Depth test.
            double d = depthbuffer.get(p.x, p.y, 0);
            double z  = 1.0 / (      t  * 1.0 / p1.z +
                              (1.0 - t) * 1.0 / p2.z);
            if (z >= d)
                continue;
            depthbuffer.set(p.x, p.y, 0, z);

            setRgba(p.x, p.y, c);
        }
    }
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class TrianglePrimitiveRasterizer : public PrimitiveRasterizer
{
public:
    class BoundingBox
    {
    public:
        BoundingBox()
        {
            min.x =  std::numeric_limits<int>::max();
            min.y =  std::numeric_limits<int>::max();
            max.x = -std::numeric_limits<int>::max();
            max.y = -std::numeric_limits<int>::max();
        }
        void update(const glm::ivec2& p)
        {
            if (p.x < min.x) min.x = p.x;
            if (p.y < min.y) min.y = p.y;
            if (p.x > max.x) max.x = p.x;
            if (p.y > max.y) max.y = p.y;
        }

        glm::ivec2 min;
        glm::ivec2 max;
    };

    TrianglePrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                                DepthFramebuffer& depthbuffer,
                                Rasterizer::NormalMode normalMode)
        : PrimitiveRasterizer(colorbuffer, depthbuffer)
        , normalMode(normalMode)
    {}

    void rasterize(const Mesh& triangleMesh,
                   const glm::dmat4& matrix,
                   const glm::dmat3& normalMatrix,
                   const glm::dvec3& lightDir)
    {
        sampler.reset();
        //if (!triangleMesh.albedoMap.empty())
        //    sampler = std::make_unique<Sampler>(QImage(QString::fromStdString(triangleMesh.albedoMap)));

        for (size_t i = 0; i < triangleMesh.indices.size(); i += 3)
        {
            unsigned i1 = triangleMesh.indices[i + 0];
            unsigned i2 = triangleMesh.indices[i + 1];
            unsigned i3 = triangleMesh.indices[i + 2];
            Triangle tri;
            tri.p1 = triangleMesh.vertices[i1];
            tri.p2 = triangleMesh.vertices[i2];
            tri.p3 = triangleMesh.vertices[i3];

            rasterize(tri, matrix, normalMatrix, lightDir);
        }
    }

    void rasterize(const Triangle& tri,
                   const glm::dmat4& matrix,
                   const glm::dmat3& normalMatrix,
                   const glm::dvec3& lightDir)
    {
        const glm::dvec3 n =
            glm::normalize(glm::cross(tri.p2.position - tri.p1.position,
                                      tri.p3.position - tri.p1.position));

        // Projection
        glm::dvec3 p1 = project(matrix, tri.p1.position);
        glm::dvec3 p2 = project(matrix, tri.p2.position);
        glm::dvec3 p3 = project(matrix, tri.p3.position);

        // Viewport transform
        glm::ivec2 vpP1 = viewportTransform(p1);
        glm::ivec2 vpP2 = viewportTransform(p2);
        glm::ivec2 vpP3 = viewportTransform(p3);

        // Shade the triangle area on the viewport
        BoundingBox bb;
        bb.update(vpP1);
        bb.update(vpP2);
        bb.update(vpP3);

        for (int y = bb.min.y; y <= bb.max.y; ++y)
        for (int x = bb.min.x; x <= bb.max.x; ++x)
        {
            glm::ivec2 p(x, y);

            // Barycentric weights
            double w1 = edgeFunction(vpP2, vpP3, p);
            double w2 = edgeFunction(vpP3, vpP1, p);
            double w3 = edgeFunction(vpP1, vpP2, p);
            if (w1 < 0.0 || w2 < 0.0 || w3 < 0.0)
                continue;

            // Top-left edge rule
            glm::ivec2 edge1 = vpP2 - vpP3;
            glm::ivec2 edge2 = vpP3 - vpP1;
            glm::ivec2 edge3 = vpP1 - vpP2;
            bool overlaps = true;
            overlaps &= (w1 == 0.0 ? ((edge1.y == 0.0 && edge1.x < 0.0) ||  edge1.y < 0.0) : (w1 > 0.0));
            overlaps &= (w2 == 0.0 ? ((edge2.y == 0.0 && edge2.x < 0.0) ||  edge2.y < 0.0) : (w2 > 0.0));
            overlaps &= (w3 == 0.0 ? ((edge3.y == 0.0 && edge3.x < 0.0) ||  edge3.y < 0.0) : (w3 > 0.0));
            if (!overlaps)
                continue;

            double area = edgeFunction(vpP1, vpP2, vpP3);
            w1 /= area;
            w2 /= area;
            w3 /= area;

            // Depth test.
            double d = depthbuffer.get(x, y, 0);
            double z  = 1.0 / (w1 * 1.0 / p1.z +
                               w2 * 1.0 / p2.z +
                               w3 * 1.0 / p3.z);
            if (z >= d)
                continue;
            depthbuffer.set(x, y, 0, z);

            glm::dvec3 n1  = glm::normalize((normalMatrix * tri.p1.normal) / p1.z);
            glm::dvec3 n2  = glm::normalize((normalMatrix * tri.p2.normal) / p2.z);
            glm::dvec3 n3  = glm::normalize((normalMatrix * tri.p3.normal) / p3.z);
            glm::dvec3 normal   = n1  * w1 * z +
                                  n2  * w2 * z +
                                  n3  * w3 * z;
            if (normalMode == Rasterizer::NormalMode::Coarse)
                normal = n;
            normal = glm::normalize(normal);

            double nDotL = glm::dot(normal, -lightDir);
            nDotL = glm::clamp(nDotL, 0.0, 1.0);

            glm::dvec4 c1  = tri.p1.color / p1.z;
            glm::dvec4 c2  = tri.p2.color / p2.z;
            glm::dvec4 c3  = tri.p3.color / p3.z;
            glm::dvec4 color = c1  * w1 * z +
                               c2  * w2 * z +
                               c3  * w3 * z;

            glm::dvec2 tc1  = tri.p1.texCoord / p1.z;
            glm::dvec2 tc2  = tri.p2.texCoord / p2.z;
            glm::dvec2 tc3  = tri.p3.texCoord / p3.z;
            glm::dvec2 tc = tc1  * w1 * z +
                            tc2  * w2 * z +
                            tc3  * w3 * z;

            if (sampler)
                color = sampler->sampleRgba(tc);
            color *= nDotL;
            color.a = 1.0;

            setRgba(x, y, color);
        }
    }
private:
    double edgeFunction(const glm::ivec2& a,
                        const glm::ivec2& b,
                        const glm::ivec2& c)
    {
        const glm::dvec2 aa = a;
        const glm::dvec2 bb = b;
        const glm::dvec2 cc = c;
        return ((cc.x - aa.x) * (bb.y - aa.y) - (cc.y - aa.y) * (bb.x - aa.x));
    }

private:
    std::unique_ptr<Sampler> sampler;
    Rasterizer::NormalMode normalMode;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Rasterizer::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(int width, int height)
        : colorFramebuffer(width, height, 4)
        , depthFramebuffer(width, height, 1)
        , normalMode(NormalMode::Coarse)
    {
        view = glm::translate(glm::dmat4(1.0), glm::dvec3(0, 0, 3.0));
        projection = glm::perspective(M_PI * 0.25, 1.0, 0.1, 150.0);
        lightDir = glm::dvec3(0, 0, -1);
        updateMatrices();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void clear()
    {
        colorFramebuffer.set(0);
        depthFramebuffer.set(std::numeric_limits<double>::max());
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void updateMatrices()
    {
        const glm::dmat4 viewInv = glm::inverse(view);
        cameraMatrix = projection * viewInv;
        normalMatrix = glm::inverseTranspose(glm::dmat3(viewInv));
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawTriangleMesh(Mesh* mesh)
    {
        TrianglePrimitiveRasterizer triRast(colorFramebuffer, depthFramebuffer, normalMode);
        triRast.rasterize(*mesh, cameraMatrix, normalMatrix, lightDir);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawLineMesh(Mesh* mesh)
    {
        LinePrimitiveRasterizer linRast(colorFramebuffer, depthFramebuffer);
        linRast.rasterize(*mesh, cameraMatrix);
    }

    ColorFramebuffer colorFramebuffer;
    DepthFramebuffer depthFramebuffer;
    NormalMode normalMode;
    glm::dmat4 view;
    glm::dmat4 projection;
    glm::dmat4 cameraMatrix;
    glm::dmat3 normalMatrix;
    std::vector<Mesh*> meshes;
    glm::dvec3 lightDir;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Rasterizer::Rasterizer(int width, int height)
    : impl(std::make_shared<Impl>(width, height))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::clear()
{ impl->clear(); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setViewMatrix(const glm::dmat4& view)
{ impl->view = view; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setProjectionMatrix(const glm::dmat4& projection)
{ impl->projection = projection; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::drawTriangleMesh(Mesh *mesh)
{ impl->drawTriangleMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::drawLineMesh(Mesh *mesh)
{ impl->drawLineMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ColorFramebuffer Rasterizer::colorFramebuffer() const
{ return impl->colorFramebuffer; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
DepthFramebuffer Rasterizer::depthFramebuffer() const
{ return impl->depthFramebuffer; }


} // namespace rasterizer
} // namespace wakusei
} // namespace kuu
