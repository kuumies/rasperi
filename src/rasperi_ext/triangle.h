/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Triangle struct.
 * -----------------------------------------------------------------*/

#pragma once

#include <glm/vec3.hpp>

namespace kuu
{

struct Ray;

/* -----------------------------------------------------------------*
   A triangle in three-dimensional space.
 * -----------------------------------------------------------------*/
struct Triangle
{
    // Result of an intersection.
    struct IntersectionResult
    {
        enum class Type
        {
            Success,
            Failure
        };

        IntersectionResult();
        operator bool() const;

        Type type;
        glm::vec3 pos;
        float t = 0.0f;
    };

    // Constructs the triangle.
    Triangle();
    Triangle(const glm::vec3& p0,
             const glm::vec3& p1,
             const glm::vec3& p2);

    // Returns true if the points are [0, 0, 0].
    bool isNull() const;

    // Returns triangle normal.
    glm::vec3 normal() const;

    // Returns true if the point is on the triangle.
    bool contains(const glm::vec3& p) const;

    // Calculates the barycentring coordinates of the point
    // on the triangle.
    bool barycentric(const glm::vec3& p,
                     float& u,
                     float& v,
                     float& w) const;

    // Returns intersection result between a ray and the triangle.
    IntersectionResult intersect(const Ray& ray) const;

    // Triangle corner points.
    glm::vec3 p0, p1, p2;
};

} // namespace kuu
