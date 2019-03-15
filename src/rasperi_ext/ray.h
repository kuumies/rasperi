/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Ray struct.
 * -----------------------------------------------------------------*/

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace kuu
{

/* -----------------------------------------------------------------*
   Ray in three-dimensional space.
 * -----------------------------------------------------------------*/
struct Ray
{
    // Constructs the ray.
    Ray();
    Ray(const glm::vec3& start, const glm::vec3& direction);

    // Returns true if the start and direction are [0, 0, 0]
    bool isNull() const;

    // Returns the position along the ray at the given distance.
    glm::vec3 position(float distance) const;

    // Transforms the ray and returns the result ray.
    Ray transformed(const glm::mat4& m) const;

    glm::vec3 start;
    glm::vec3 direction;
};

} // namespace kuu
