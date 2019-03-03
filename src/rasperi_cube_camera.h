/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::CubeCamera class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <array>
#include <glm/mat4x4.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class CubeCamera
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    CubeCamera(double aspectRatio);

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dmat4 cameraMatrix(size_t face) const;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dmat4 projectionMatrix;
    std::array<glm::dmat4, 6> viewMatrices;

};

} // namespace rasperi
} // namespace kuu
