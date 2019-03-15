/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Triangle struct.
 * -----------------------------------------------------------------*/

#include "triangle.h"
#include <glm/geometric.hpp>
#include "ray.h"

namespace kuu
{

/* -----------------------------------------------------------------*
   Triangle::IntersectionResult
 * -----------------------------------------------------------------*/

Triangle::IntersectionResult::IntersectionResult()
    : type(Type::Failure)
{}

Triangle::IntersectionResult::operator bool() const
{
    return type == Type::Success;
}

/* -----------------------------------------------------------------*
   Triangle
 * -----------------------------------------------------------------*/

Triangle::Triangle()
{}

Triangle::Triangle(const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& p3)
    : p0(p1)
    , p1(p2)
    , p2(p3)
{}

bool Triangle::isNull() const
{
    return p0 == glm::vec3(0.0) &&
           p1 == glm::vec3(0.0) &&
            p2 == glm::vec3(0.0);
}

glm::vec3 Triangle::normal() const
{
    glm::vec3 u = p1 - p0;
    glm::vec3 v = p2 - p0;
    glm::vec3 n = glm::cross(u, v);
    n = glm::normalize(n);
    return n;
}

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
    glm::vec3 v[3] = { p0, p1, p2 };

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

/* -----------------------------------------------------------------*
   Algorithm from Didier Badouel, Graphics Gems I, pp 390-393
   From 3D primer book.
 * -----------------------------------------------------------------*/
Triangle::IntersectionResult Triangle::intersect(
    const Ray& ray) const
{
    float minT = 1.0;

    // Compute clockwise edge vectors.
    glm::vec3 e1 = p1 - p0;
    glm::vec3 e2 = p2 - p1;

    // Compute surface normal. (Unnormalized)
    glm::vec3 n = glm::cross(e1, e2);

    // Compute gradient, which tells us how steep of an angle
    // we are approaching the *front* side of the triangle
    float dot = glm::dot(n, ray.direction);

    // Check for a ray that is parallel to the triangle or not
    // pointing toward the front face of the triangle.
    //
    // Note that this also will reject degenerate triangles and
    // rays as well. We code this in a very particular
    // way so that NANs will bail here. (This does not
    // behave the same as "dot >= 0.0f" when NANs are involved.)
    if (!(dot < 0.0f))
        return IntersectionResult();

    // Compute d value for the plane equation. We will
    // use the plane equation with d on the right side:
    //
    // Ax + By + Cz = d
    float d = glm::dot(n, p0);

    // Compute parametric point of intersection with the plane
    // containing the triangle, checking at the earliest
    // possible stages for trivial rejection
    float t = d - glm::dot(n, ray.start);

    // Is ray origin on the backside of the polygon? Again,
    // we phrase the check so that NANs will bail
    if (!(t <= 0.0f))
        return IntersectionResult();

    // Closer intersection already found? (Or does
    // ray not reach the plane?)
    //
    // since dot < 0:
    //
    // t/dot > minT
    //
    // is the same as
    //
    // t < dot*minT
    //
    // (And then we invert it for NAN checking...)
    if (!(t >= dot*minT))
        return IntersectionResult();

    // OK, ray intersects the plane. Compute actual parametric
    // point of intersection
    t /= dot;
    assert(t >= 0.0f);
    assert(t <= minT);

    // Compute 3D point of intersection
    glm::vec3 p = ray.start + ray.direction * t;

    // Find dominant axis to select which plane
    // to project onto, and compute u's and v's
    float u0, u1, u2;
    float v0, v1, v2;
    if (fabs(n.x) > fabs(n.y))
    {
        if (fabs(n.x) > fabs(n.z))
        {
            u0 = p.y - p0.y;
            u1 = p1.y - p0.y;
            u2 = p2.y - p0.y;
            v0 = p.z - p0.z;
            v1 = p1.z - p0.z;
            v2 = p2.z - p0.z;
        }
        else
        {
            u0 = p.x - p0.x;
            u1 = p1.x - p0.x;
            u2 = p2.x - p0.x;
            v0 = p.y - p0.y;
            v1 = p1.y - p0.y;
            v2 = p2.y - p0.y;
        }
    }
    else
    {
        if (fabs(n.y) > fabs(n.z))
        {
            u0 = p.x - p0.x;
            u1 = p1.x - p0.x;
            u2 = p2.x - p0.x;
            v0 = p.z - p0.z;
            v1 = p1.z - p0.z;
            v2 = p2.z - p0.z;
        }
        else
        {
            u0 = p.x - p0.x;
            u1 = p1.x - p0.x;
            u2 = p2.x - p0.x;
            v0 = p.y - p0.y;
            v1 = p1.y - p0.y;
            v2 = p2.y - p0.y;
        }
    }

    // Compute denominator, check for invalid
    float temp = u1 * v2 - v1 * u2;
    if (!(temp != 0.0f))
        return IntersectionResult();

    temp = 1.0f / temp;

    // Compute barycentric coords, checking for out-of-range
    // at each step
    float alpha = (u0 * v2 - v0 * u2) * temp;
    if (!(alpha >= 0.0f))
    return IntersectionResult();

    float beta = (u1 * v0 - v1 * u0) * temp;
    if (!(beta >= 0.0f))
        return IntersectionResult();

    float gamma = 1.0f - alpha - beta;
    if (!(gamma >= 0.0f))
        return IntersectionResult();

    IntersectionResult res;
    res.type = IntersectionResult::Type::Success;
    res.t    = t;
    res.pos  = ray.position(t);
    return res;
}

} // namespace kuu
