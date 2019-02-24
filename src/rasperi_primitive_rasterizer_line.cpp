/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::LinePrimitiveRasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_primitive_rasterizer.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct LinePrimitiveRasterizer::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(LinePrimitiveRasterizer* self)
        : self(self)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void rasterize(const Vertex& v1,
                   const Vertex& v2,
                   const glm::dmat4& matrix)
    {
        // Projection
        glm::dvec3 p1 = self->project(matrix, v1.position);
        glm::dvec3 p2 = self->project(matrix, v2.position);

        // Viewport transform
        glm::dvec2 vpP1 = self->viewportTransform(p1);
        glm::dvec2 vpP2 = self->viewportTransform(p2);
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
            double d = self->depthbuffer.get(p.x, p.y, 0);
            double z  = 1.0 / (      t  * 1.0 / p1.z +
                              (1.0 - t) * 1.0 / p2.z);
            if (z >= d)
                continue;

            self->depthbuffer.set(p.x, p.y, 0, z);
            self->setRgba(p.x, p.y, c);
        }
    }

    LinePrimitiveRasterizer* self = nullptr;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
LinePrimitiveRasterizer::LinePrimitiveRasterizer(ColorFramebuffer& colorbuffer,
                        DepthFramebuffer& depthbuffer)
    : PrimitiveRasterizer(colorbuffer, depthbuffer)
    , impl(std::make_shared<Impl>(this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void LinePrimitiveRasterizer::rasterize(const Mesh& m, const glm::dmat4& matrix)
{
    for (size_t i = 0; i < m.indices.size(); i += 2)
    {
        unsigned i1 = m.indices[i + 0];
        unsigned i2 = m.indices[i + 1];
        Vertex v1 = m.vertices[i1];
        Vertex v2 = m.vertices[i2];

        impl->rasterize(v1, v2, matrix);
    }
}

} // namespace rasperi
} // namespace kuu
