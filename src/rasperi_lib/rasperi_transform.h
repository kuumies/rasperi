/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Transform class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Transform
{
public:
    glm::dmat4 matrix() const;

    glm::dvec3 position;
    glm::dquat rotation;
    glm::dvec3 scale = glm::dvec3(1.0);
};

} // namespace rasperi
} // namespace kuu
