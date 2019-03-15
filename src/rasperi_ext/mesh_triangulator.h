/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::mesh_triangulator namespace.
 * -----------------------------------------------------------------*/

#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include "triangle.h"

namespace kuu
{

namespace mesh_triangulator
{

/* -----------------------------------------------------------------*
   Triangulates a polygon.
 * -----------------------------------------------------------------*/
std::vector<Triangle> polygons(std::vector<glm::vec3> polygon);


} // namespace mesh_triangulator
} // namespace kuu
