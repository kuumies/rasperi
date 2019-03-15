/* -----------------------------------------------------------------*
  Antti Jumpponen <kuumies@gmail.com>
  The implementation of kuu::BoundingBox struct.
 * -----------------------------------------------------------------*/

#include "bounding_box.h"
#include <limits>
#include <glm/common.hpp>

namespace kuu
{

BoundingBox::BoundingBox()
{
    reset();
}

BoundingBox::BoundingBox(const glm::vec3& min,
                         const glm::vec3& max)
    : min(min)
    , max(max)
{}

void BoundingBox::update(const glm::vec3& point)
{
    for (int i = 0; i < 3; ++i)
    {
        max[i] = glm::max(point[i], max[i]);
        min[i] = glm::min(point[i], min[i]);
    }
}

void BoundingBox::update(const BoundingBox& bb)
{
    update(bb.min);
    update(bb.max);
}

glm::vec3 BoundingBox::center() const
{
    return (min + max) / 2.0f;
}

glm::vec3 BoundingBox::size() const
{
    return max - min;
}

std::vector<glm::vec3> BoundingBox::corners() const
{
    std::vector<glm::vec3> out;
    // bottom plane
    out.push_back(min);
    out.push_back(glm::vec3(min.x, min.y, max.z));
    out.push_back(glm::vec3(max.x, min.y, max.z));
    out.push_back(glm::vec3(max.x, min.y, min.z));
    // top plane
    out.push_back(max);
    out.push_back(glm::vec3(min.x, max.y, max.z));
    out.push_back(glm::vec3(max.x, max.y, max.z));
    out.push_back(glm::vec3(max.x, max.y, min.z));
    return out;
}

std::vector<Plane> BoundingBox::innerPlanes() const
{
    return { Plane(min, glm::vec3( 1,  0,  0)),
             Plane(max, glm::vec3(-1,  0,  0)),
             Plane(min, glm::vec3( 0,  1,  0)),
             Plane(max, glm::vec3( 0, -1,  0)),
             Plane(min, glm::vec3( 0,  0,  1)),
             Plane(max, glm::vec3( 0,  0, -1)) };
}

void BoundingBox::reset()
{
    min = glm::vec3(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max());

    max = glm::vec3(
        -std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity());
}

bool BoundingBox::contains(const glm::vec3& p) const
{
    if (p.x < min.x || p.x > max.x)
        return false;

    if (p.y < min.y || p.y > max.y)
        return false;

    if (p.z < min.z || p.z > max.z)
        return false;

    return true;
}

void BoundingBox::transform(const glm::mat4& m)
{
    min = glm::vec3(m * glm::vec4(min, 1.0f));
    max = glm::vec3(m * glm::vec4(max, 1.0f));
}

BoundingBox BoundingBox::transformed(const glm::mat4& m) const
{
    BoundingBox out(*this);
    out.transform(m);
    return out;
}

} // namespace kuu
