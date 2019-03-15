/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::TriangleClipper class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
//#include <vector>
//#include "triangle.h"
#include "rasperi_lib/rasperi_mesh.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class TriangleClipper
{
public:
    TriangleClipper(const glm::dmat4& cameraMatrix,
                    const glm::vec4& viewport);

    std::vector<Triangle> clip(const Triangle& tri) const;

    //std::vector<kuu::Triangle> ndcClip(std::vector<glm::vec3> tri);
    //std::vector<kuu::Triangle> polygons(std::vector<Vertex> polygon);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
