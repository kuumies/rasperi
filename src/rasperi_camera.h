/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::Camera class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class Camera
{
public:
    glm::dmat4 viewMatrix() const;
    glm::dmat4 projectionMatrix() const;
    glm::dmat4 cameraMatrix() const;

    // Transform
    glm::dvec3 position  = glm::dvec3(0, 0, 5);
    glm::dquat rotation = glm::dquat(1.0, 0.0, 0.0, 0.0);

    // Projection
    double fieldOfView = 45.0;
    double aspectRatio = 1.0;
    double nearPlane   = 0.1;
    double farPlane    = 75.0;
};

} // namespace rasperi
} // namespace kuu
