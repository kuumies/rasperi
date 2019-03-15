/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Plane struct.
 * -----------------------------------------------------------------*/

#pragma once

#include <glm/vec3.hpp>

namespace kuu
{

struct Ray;

/* -----------------------------------------------------------------*
   Infinite plane in three-dimensional space.
 * -----------------------------------------------------------------*/
struct Plane
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

    // Constructs the plane.
    Plane();
    Plane(const glm::vec3& point, const glm::vec3& normal);

    // Returns true if the point and normal are [0, 0, 0].
    bool isNull() const;

    // Returns the closest distance to ray.
    float distance(const Ray &ray) const;
    // Returns the closest distance to point.
    float distance(const glm::vec3& p) const;

    // Projects a point into plane.
    glm::vec3 project(const glm::vec3& p) const;

    // Returns an intersection result between plane and ray.
    IntersectionResult intersect(const Ray& ray) const;

    glm::vec3 point;
    glm::vec3 normal;
};

} // namespace kuu
