/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Plane struct.
 * -----------------------------------------------------------------*/

#include "plane.h"
#include <glm/geometric.hpp>
#include "ray.h"

namespace kuu
{

/* -----------------------------------------------------------------*
   Plane::IntersectionResult
 * -----------------------------------------------------------------*/

Plane::IntersectionResult::IntersectionResult()
    : type(Type::Failure)
{}

Plane::IntersectionResult::operator bool() const
{
    return type == Type::Success;
}

/* -----------------------------------------------------------------*
   Plane
 * -----------------------------------------------------------------*/

Plane::Plane()
{}

Plane::Plane(const glm::vec3& point, const glm::vec3& normal)
    : point(point)
    , normal(normal)
{}

bool Plane::isNull() const
{
    return point == glm::vec3(0.0) && normal == glm::vec3(0.0);
}

float Plane::distance(const Ray& ray) const
{
    float dot = glm::dot(point - ray.start, normal);
    float div = glm::dot(ray.direction, normal);
    return dot / div;
}

float Plane::distance(const glm::vec3& p) const
{
    glm::vec3 toPoint = p -point;
    return glm::dot(normal, toPoint);
}

glm::vec3 Plane::project(const glm::vec3& p) const
{
    return p - normal * distance(p);
}

Plane::IntersectionResult Plane::intersect(const Ray &ray) const
{
    // Calculate the closest distance from the ray origo to plane.
    float dot = glm::dot(point - ray.start, normal);
    float div = glm::dot(ray.direction, normal);
    float distance = dot / div;

    // If the distance is < 0 then the ray points away from the plane
    if (distance < 0.0f)
        return IntersectionResult();

    // Calculate the intersection position
    IntersectionResult res;
    res.type = IntersectionResult::Type::Success;
    res.pos = ray.position(distance);
    res.t = distance;
    return res;
}

} // namespace kuu
