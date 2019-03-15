/* -----------------------------------------------------------------*
  Antti Jumpponen <kuumies@gmail.com>
  The implementation of kuu::mesh_triangulator namespace.
 * -----------------------------------------------------------------*/

#include "mesh_triangulator.h"
#include <glm/common.hpp>
#include <glm/geometric.hpp>

namespace kuu
{
namespace mesh_triangulator
{

// Returns angle in radians between two edges of a point.
// See: https://stackoverflow.com/questions/43493711/
//      the-angle-between-two-3d-vectors-with-a-result-range-0-360
float angleBetweenThreeVertices(std::vector<glm::vec3> vertices,
                                glm::vec3 up)
{
    glm::vec3 v1 = vertices[1] - vertices[0];
    glm::vec3 v2 = vertices[2] - vertices[1];
    glm::vec3 cross = glm::cross(v1, v2);
    float dot = glm::dot(v1, v2);
    float angle = std::atan2(glm::length(cross), dot);

    float test = glm::dot(up, cross);
    if (test < 0.0)
        angle = -angle;
    return angle;
}

std::vector<Triangle> polygons(
    std::vector<glm::vec3> poly)
{
    // At least four vertices required
    if (poly.size() < 3)
        return {};

    // Polygon normal for using correct angle sign.
    Triangle tri(poly[0], poly[1], poly[2]);
    glm::vec3 n = tri.normal();
    //vec3.fromValues(0, 0, 1)

    std::vector<Triangle> triangles;
    for (int i = 0; i < poly.size(); ++i)
    {
        // Find adjacent vertices
        int i0 = i > 0 ? i - 1 : int(poly.size()) - 1;
        int i1 = i;
        int i2 = i < (poly.size() - 1) ? i + 1 : 0;
        glm::vec3 v0 = poly[i0];
        glm::vec3 v1 = poly[i1];
        glm::vec3 v2 = poly[i2];

        // Vertex must be convex
        float angle = angleBetweenThreeVertices({ v0, v1, v2 }, n);
        if (angle < 0.0 || angle >= M_PI)
            continue;

        // Vertex must be a eartip
        Triangle tri(v0, v1, v2);
        bool isEarTip = true;
        for (int j = 0; j < poly.size(); ++j)
        {
            // Skip triangle indices
            if (j == i0 || j == i1 || j == i2)
                continue;

            // Test vertex
            glm::vec3 v = poly[j];
            if (tri.contains(v))
            {
                isEarTip = false;
                break;
            }
        }

        if (isEarTip)
        {
            // Add a new triangle
            triangles.push_back(Triangle(v1, v2, v0));
            // Remove  the ear tip vertex
            poly.erase(poly.begin() + i1);
            //poly.splice(i1, 1);
            // Start vertices  from the beginning
            i = -1;
            // Stop if all triangles have been found
            if (poly.size() < 3)
                break;
        }
    }

    // Return triangles
    return triangles;
}

} // namespace mesh_triangulator
} // namespace kuu
