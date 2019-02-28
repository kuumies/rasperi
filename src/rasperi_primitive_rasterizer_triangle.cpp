/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::TrianglePrimitiveRasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_primitive_rasterizer.h"
#include <QtCore/QDebug>
#include <QtCore/QTime>
#include "rasperi_material.h"
#include "rasperi_sampler.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct TrianglePrimitiveRasterizer::Impl
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

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(Rasterizer::NormalMode normalMode,
         TrianglePrimitiveRasterizer* self)
        : self(self)
        , normalMode(normalMode)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void rasterize(const Triangle& tri,
                   const glm::dmat4& cameraMatrix,
                   const glm::dmat4& modelMatrix,
                   const glm::dmat3& normalMatrix,
                   const glm::dvec3& lightDir,
                   const glm::dvec3& cameraPos,
                   const Material& material)
    {
        const glm::dvec3 triNormal =
            glm::normalize(glm::cross(tri.p2.position - tri.p1.position,
                                      tri.p3.position - tri.p1.position));

        // Projection
        glm::dvec3 p1 = self->project(cameraMatrix, tri.p1.position);
        glm::dvec3 p2 = self->project(cameraMatrix, tri.p2.position);
        glm::dvec3 p3 = self->project(cameraMatrix, tri.p3.position);

        // Viewport transform
        glm::ivec2 vpP1 = self->viewportTransform(p1);
        glm::ivec2 vpP2 = self->viewportTransform(p2);
        glm::ivec2 vpP3 = self->viewportTransform(p3);

        // Shade the triangle area on the viewport
        BoundingBox bb;
        bb.update(vpP1);
        bb.update(vpP2);
        bb.update(vpP3);

        #pragma omp parallel for
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
            double d = self->depthbuffer.get(x, y, 0);
            double z  = 1.0 / (w1 * 1.0 / p1.z +
                               w2 * 1.0 / p2.z +
                               w3 * 1.0 / p3.z);
            if (z >= d)
                continue;
            self->depthbuffer.set(x, y, 0, z);

            Vertex vertex = interpolatedVertex(tri,
                                               w1, w2, w3,
                                               p1.z, p2.z, p3.z,
                                               z,
                                               modelMatrix,
                                               normalMatrix);


            if (normalMode == Rasterizer::NormalMode::Coarse)
                vertex.normal = triNormal;
            vertex.normal = glm::normalize(vertex.normal);

            if (material.normalSampler.isValid())
            {
                glm::dmat3 tbn = glm::dmat3(vertex.tangent,
                                            vertex.bitangent,
                                            vertex.normal);

                vertex.normal = material.normalSampler.sampleRgba(vertex.texCoord);
                vertex.normal = normalize(vertex.normal * 2.0 - 1.0);
                vertex.normal = tbn * vertex.normal;
                vertex.normal = normalize(vertex.normal);
            }

            glm::dvec3 n = glm::normalize(vertex.normal);
            glm::dvec3 l = glm::normalize(-lightDir);
            glm::dvec3 v = glm::normalize(cameraPos - vertex.position);
            glm::dvec3 r = glm::reflect(l, vertex.normal);

            glm::dvec4 color = litVertex(vertex, material,
                                         n, v, l, r);
            self->setRgba(x, y, color);
        }
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    double edgeFunction(const glm::ivec2& a,
                        const glm::ivec2& b,
                        const glm::ivec2& c)
    {
        const glm::dvec2 aa = a;
        const glm::dvec2 bb = b;
        const glm::dvec2 cc = c;
        return ((cc.x - aa.x) * (bb.y - aa.y) - (cc.y - aa.y) * (bb.x - aa.x));
    }

    /* ------------------------------------------------------------ *
       Interpolates triangle vertex
        * transform vertex position, normal, binormal and tangets to world space
        * applies perspective correction to values
        * interpolates vertex based on barycentric weights
     * ------------------------------------------------------------ */
    Vertex interpolatedVertex(
        const Triangle& tri,
        double w1, double w2, double w3,
        double z1, double z2, double z3,
        double z,
        const glm::dmat4& modelMatrix,
        const glm::dmat3& normalMatrix) const
    {
        Vertex out;

        glm::dvec3 p1 = modelMatrix * glm::dvec4(tri.p1.position, 1.0) / z1;
        glm::dvec3 p2 = modelMatrix * glm::dvec4(tri.p2.position, 1.0) / z2;
        glm::dvec3 p3 = modelMatrix * glm::dvec4(tri.p3.position, 1.0) / z3;
        out.position  = p1  * w1 * z +
                        p2  * w2 * z +
                        p3  * w3 * z;

        glm::dvec4 c1 = tri.p1.color / z1;
        glm::dvec4 c2 = tri.p2.color / z2;
        glm::dvec4 c3 = tri.p3.color / z2;
        out.color     = c1  * w1 * z +
                        c2  * w2 * z +
                        c3  * w3 * z;

        glm::dvec2 tc1  = tri.p1.texCoord / z1;
        glm::dvec2 tc2  = tri.p2.texCoord / z2;
        glm::dvec2 tc3  = tri.p3.texCoord / z3;
        out.texCoord    = tc1  * w1 * z +
                          tc2  * w2 * z +
                          tc3  * w3 * z;

        glm::dvec3 n1  = glm::normalize((normalMatrix * tri.p1.normal) / z1);
        glm::dvec3 n2  = glm::normalize((normalMatrix * tri.p2.normal) / z2);
        glm::dvec3 n3  = glm::normalize((normalMatrix * tri.p3.normal) / z3);
        out.normal     = n1  * w1 * z +
                         n2  * w2 * z +
                         n3  * w3 * z;

        glm::dvec3 t1  =  glm::normalize(normalMatrix * tri.p1.tangent) / z1;
        glm::dvec3 t2  =  glm::normalize(normalMatrix * tri.p2.tangent) / z2;
        glm::dvec3 t3  =  glm::normalize(normalMatrix * tri.p3.tangent) / z3;
        out.tangent    = t1  * w1 * z +
                         t2  * w2 * z +
                         t3  * w3 * z;

        glm::dvec3 b1  =  glm::normalize(normalMatrix * tri.p1.bitangent) / z1;
        glm::dvec3 b2  =  glm::normalize(normalMatrix * tri.p2.bitangent) / z2;
        glm::dvec3 b3  =  glm::normalize(normalMatrix * tri.p3.bitangent) / z3;
        out.bitangent  = b1  * w1 * z +
                         b2  * w2 * z +
                         b3  * w3 * z;
        // re-orthogonalize T with respect to N
        out.tangent   = normalize(out.tangent- dot(out.tangent, out.normal) * out.normal);
        out.bitangent = cross(out.normal, out.tangent);

        return out;
    }

    glm::dvec4 litVertex(const Vertex& vertex,
                         const Material& material,
                         const glm::dvec3& n,
                         const glm::dvec3& v,
                         const glm::dvec3& l,
                         const glm::dvec3& r) const
    {
        double nDotL = glm::dot(n, l);
        nDotL = glm::clamp(nDotL, 0.0, 1.0);

        double vDotR = glm::dot(v, r);
        vDotR = glm::clamp(vDotR, 0.0, 1.0);

        glm::dvec3 ambient = material.ambient;
        if (material.ambientSampler.isValid())
            ambient = material.ambientSampler.sampleRgba(vertex.texCoord);

        glm::dvec3 diffuse = material.diffuse;
        if (material.diffuseFromVertex)
            diffuse = vertex.color;
        if (material.diffuseSampler.isValid())
            diffuse = material.diffuseSampler.sampleRgba(vertex.texCoord);
        diffuse *= nDotL;

        double specularPower = material.specularPower;
        if (material.specularPowerSampler.isValid())
            specularPower = material.specularPowerSampler.sampleRgba(vertex.texCoord).x;

        glm::dvec3 specular = material.specular;
        if (material.specularSampler.isValid())
            specular = material.specularSampler.sampleRgba(vertex.texCoord);
        specular = specular * std::pow(vDotR, specularPower);

        return glm::dvec4(diffuse + specular, 1.0);
    }

    TrianglePrimitiveRasterizer* self;
    Rasterizer::NormalMode normalMode;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
TrianglePrimitiveRasterizer::TrianglePrimitiveRasterizer(
        ColorFramebuffer& colorbuffer,
        DepthFramebuffer& depthbuffer,
        Rasterizer::NormalMode normalMode)
    : PrimitiveRasterizer(colorbuffer, depthbuffer)
    , impl(std::make_shared<Impl>(normalMode, this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void TrianglePrimitiveRasterizer::rasterize(
        const Mesh& triangleMesh,
        const glm::dmat4& cameraMatrix,
        const glm::dmat4& modelMatrix,
        const glm::dmat3& normalMatrix,
        const glm::dvec3& lightDir,
        const glm::dvec3& cameraPos,
        const Material& material)
{
    QTime timer;
    timer.start();

    for (size_t i = 0; i < triangleMesh.indices.size(); i += 3)
    {
        unsigned i1 = triangleMesh.indices[i + 0];
        unsigned i2 = triangleMesh.indices[i + 1];
        unsigned i3 = triangleMesh.indices[i + 2];
        if (i1 >= triangleMesh.vertices.size() ||
            i2 >= triangleMesh.vertices.size() ||
            i3 >= triangleMesh.vertices.size())
        {
            continue;
        }

        Triangle tri;
        tri.p1 = triangleMesh.vertices[i1];
        tri.p2 = triangleMesh.vertices[i2];
        tri.p3 = triangleMesh.vertices[i3];

        impl->rasterize(tri, cameraMatrix, modelMatrix, normalMatrix, lightDir, cameraPos, material);
    }

    qDebug() << __FUNCTION__ << timer.elapsed() << "ms";
}

} // namespace rasperi
} // namespace kuu
