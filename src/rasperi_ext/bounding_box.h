/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::BoundingBox struct.
 * -----------------------------------------------------------------*/

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include "plane.h"

namespace kuu
{

/* -----------------------------------------------------------------*
   A bounding box of set of points.
 * -----------------------------------------------------------------*/
struct BoundingBox
{
    // Constructs the bounding box. The bounding box is infinite.
    BoundingBox();
    BoundingBox(const glm::vec3& min, const glm::vec3& max);

    // Updates the bounding box to contain the point.
    void update(const glm::vec3& point);
    // Updates the bounding box to contain another bounding box.
    void update(const BoundingBox& bb);

    // Returns the boundig box center point.
    glm::vec3 center() const;
    // Returns the bounding box size.
    glm::vec3 size() const;
    // Returns the bounding box corners.
    std::vector<glm::vec3> corners() const;
    // Returns the bounding box planes pointing inside the box.
    std::vector<Plane> innerPlanes() const;

    // Resets the bounding box. The box is now infinite.
    void reset();

    // Returns true if the bounding box contains the given in point.
    bool contains(const glm::vec3& p) const;

    // Transform
    void transform(const glm::mat4& m);
    BoundingBox transformed(const glm::mat4& m) const;

    // Minimum value.
    glm::vec3 min;
    // Maximum value.
    glm::vec3 max;
};

} // namespace kuu
