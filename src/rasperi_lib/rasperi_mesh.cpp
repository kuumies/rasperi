/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::struct class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_mesh.h"
#include <glm/geometric.hpp>

namespace kuu
{
namespace rasperi
{

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
Triangle::Triangle()
{}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
Triangle::Triangle(const Vertex& p1, const Vertex& p2, const Vertex& p3)
    : p1(p1)
    , p2(p2)
    , p3(p3)
{}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
glm::vec3 Triangle::normal() const
{
    glm::vec3 u = p2.position - p1.position;
    glm::vec3 v = p3.position - p1.position;
    glm::vec3 n = glm::cross(u, v);
    n = glm::normalize(n);
    return n;
}

/* -----------------------------------------------------------------*
 * -----------------------------------------------------------------*/
bool Triangle::contains(const glm::vec3& p) const
{
    float u, v, w;
    return barycentric(p, u, v, w);
}

/* -----------------------------------------------------------------*
   From 3D primer book.
 * -----------------------------------------------------------------*/
bool Triangle::barycentric(const glm::vec3& p,
                           float& uu,
                           float& vv,
                           float& ww) const
{
    glm::vec3 v[3] = { p1.position, p2.position, p3.position };

    // First, compute two clockwise edge vectors
    glm::vec3 d1 = v[1] - v[0];
    glm::vec3 d2 = v[2] - v[1];

    // Compute surface normal using cross product. In many cases
    // this step could be skipped, since we would have the surface
    // normal precomputed. We do not need to normalize it, although
    // if a precomputed normal was normalized, it would be OK.
    glm::vec3 n = glm::cross(d1, d2);

    // Locate dominant axis of normal, and select plane of projection
    float u1, u2, u3, u4;
    float v1, v2, v3, v4;
    if ((fabs(n.x) >= fabs(n.y)) && (fabs(n.x) >= fabs(n.z)))
    {
        // Discard x, project onto yz plane
        u1 = v[0].y - v[2].y;
        u2 = v[1].y - v[2].y;
        u3 = p.y - v[0].y;
        u4 = p.y - v[2].y;
        v1 = v[0].z - v[2].z;
        v2 = v[1].z - v[2].z;
        v3 = p.z - v[0].z;
        v4 = p.z - v[2].z;
    }
    else if (fabs(n.y) >= fabs(n.z))
    {
        // Discard y, project onto xz plane
        u1 = v[0].z - v[2].z;
        u2 = v[1].z - v[2].z;
        u3 = p.z - v[0].z;
        u4 = p.z - v[2].z;
        v1 = v[0].x - v[2].x;
        v2 = v[1].x - v[2].x;
        v3 = p.x - v[0].x;
        v4 = p.x - v[2].x;
    }
    else
    {
        u1 = v[0].x - v[2].x;
        u2 = v[1].x - v[2].x;
        u3 = p.x - v[0].x;
        u4 = p.x - v[2].x;
        v1 = v[0].y - v[2].y;
        v2 = v[1].y - v[2].y;
        v3 = p.y - v[0].y;
        v4 = p.y - v[2].y;
    }

    // Compute denominator, check for invalid
    float denom = v1 * u2 - v2 * u1;
    if (denom == 0.0f)
    {
        // Bogus triangle - probably triangle has zero area
        return false;
    }

    // Compute barycentric coordinates
    float oneOverDenom = 1.0f / denom;
    uu = (v4*u2 - v2*u4) * oneOverDenom;
    if (uu < 0.0f || uu > 1.0f)
        return false;

    vv = (v1*u3 - v3*u1) * oneOverDenom;
    if (vv < 0.0f || vv > 1.0f)
        return false;

    ww = 1.0f - uu - vv;
    if (ww < 0.0f || ww > 1.0f)
        return false;

    float sum = uu + vv + ww;
    return sum >= 0.0f && sum <= 1.0f;
}

} // namespace rasperi
} // namespace kuu
