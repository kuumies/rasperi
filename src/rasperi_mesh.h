/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::Mesh class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Vertex
{
    glm::dvec3 position;
    glm::dvec2 texCoord;
    glm::dvec3 normal;
    glm::dvec4 color;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Triangle
{
    Vertex p1;
    Vertex p2;
    Vertex p3;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Mesh
{
    std::vector<unsigned> indices;
    std::vector<Vertex> vertices;
};

} // namespace rasperi
} // namespace kuu
