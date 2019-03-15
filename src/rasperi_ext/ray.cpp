/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Ray struct.
 * -----------------------------------------------------------------*/

#include "ray.h"

namespace kuu
{

Ray::Ray()
{}

Ray::Ray(const glm::vec3& start, const glm::vec3& direction)
    : start(start)
    , direction(direction)
{}

bool Ray::isNull() const
{
    return start     == glm::vec3(0.0) &&
           direction == glm::vec3(0.0);
}

glm::vec3 Ray::position(float distance) const
{
    return start + direction * distance;
}

Ray Ray::transformed(const glm::mat4& m) const
{
    Ray out;
    out.start     = glm::vec3(m * glm::vec4(start,     1.0f));
    out.direction = glm::vec3(m * glm::vec4(direction, 0.0f));
    return out;
}

} // namespace kuu
