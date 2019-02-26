/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::Camera class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_camera.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dmat4 Camera::viewMatrix() const
{
    glm::dmat4 viewMatrix = glm::mat4(1.0);
    viewMatrix *= glm::mat4_cast(rotation);
    viewMatrix = glm::translate(viewMatrix, -position);
    return viewMatrix;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dmat4 Camera::projectionMatrix() const
{
    return glm::perspective(
        glm::radians(fieldOfView),
        aspectRatio,
        nearPlane,
        farPlane);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dmat4 Camera::cameraMatrix() const
{ return projectionMatrix() * viewMatrix(); }

} // namespace rasperi
} // namespace kuu
