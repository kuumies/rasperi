/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Projector class.
 * -----------------------------------------------------------------*/

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include "ray.h"

namespace kuu
{

/* -----------------------------------------------------------------*
   Project a point from the world space into the viewport space
   and vice-versa.
 * -----------------------------------------------------------------*/
class Projector
{
public:
    // Constructs the projector.
    Projector(const glm::mat4& camera,
              const glm::vec4& viewport);

    // Projects a 3D world point onto viewport.
    glm::vec3 project(const glm::vec3& point) const;
    glm::vec3 project(float x, float y, float z) const;

    // Unprojects a 2D on the viewport into 3D world.
    glm::vec3 unproject(const glm::vec3& point,
                        bool topDown = true) const;
    glm::vec3 unproject(float x, float y, float z,
                        bool topDown = true) const;

    // Constructs a ray from the viewport point into world space.
    Ray viewportRay(const glm::vec2& pos) const;

private:
    const glm::mat4 camera_;
    const glm::vec4 viewport;
};

} // namespace kuu
