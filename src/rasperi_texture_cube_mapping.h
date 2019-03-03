/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::texture_cube_mapping namspace.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace kuu
{
namespace rasperi
{
namespace texture_cube_mapping
{

/* ------------------------------------------------------------ *
 * ------------------------------------------------------------ */
enum class Face
{
    PositiveX,
    NegativeX,
    PositiveY,
    NegativeY,
    PositiveZ,
    NegativeZ
};

/* ------------------------------------------------------------ *
 * ------------------------------------------------------------ */
struct TextureCoordinate
{
    int faceIndex;
    glm::dvec2 uv;
};

/* ------------------------------------------------------------ *
 * ------------------------------------------------------------ */
glm::dvec3 mapTextureCoordinate(const TextureCoordinate& tc);

/* ------------------------------------------------------------ *
 * ------------------------------------------------------------ */
TextureCoordinate mapPoint(const glm::dvec3& point);

} // namespace texture_cube_mapping
} // namespace rasperi
} // namespace kuu
