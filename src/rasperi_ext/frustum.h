/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Frustum struct.
 * -----------------------------------------------------------------*/

#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

namespace kuu
{

/* -----------------------------------------------------------------*
   Camera frustum.
 * -----------------------------------------------------------------*/
struct Frustum
{
    Frustum(const glm::mat4& camera,
            const glm::vec4& viewport);

    glm::vec3 centroid() const;
    glm::vec3 farCenter() const;
    glm::vec3 nearCenter() const;

    std::vector<glm::vec3> corners;
};

} // namespace kuu
