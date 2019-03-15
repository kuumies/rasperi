/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Mesh struct.
 * ---------------------------------------------------------------- */
 
#pragma once

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
    Vertex(){}
    Vertex(glm::dvec3 position) : position(position){}
    glm::dvec3 position;
    glm::dvec2 texCoord;
    glm::dvec3 normal;
    glm::dvec3 tangent;
    glm::dvec3 bitangent;
    glm::dvec4 color;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Triangle
{
    Triangle();
    Triangle(const Vertex& p0,
             const Vertex& p1,
             const Vertex& p2);

    glm::vec3 normal() const;

    bool contains(const glm::vec3& p) const;

    bool barycentric(const glm::vec3& p,
                     float& u,
                     float& v,
                     float& w) const;

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
