/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::CubeCamera class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_cube_camera.h"
#include <glm/gtc/quaternion.hpp>

namespace kuu
{
namespace rasperi
{

/* ------------------------------------------------------------ *
 * ------------------------------------------------------------ */
CubeCamera::CubeCamera(double aspectRatio)
{
    // --------------------------------------------------------
    // Create a camera for each cubemap face

    std::array<glm::dquat, 6> rotations =
    {
        // pos x
        glm::angleAxis(glm::radians(-90.0), glm::dvec3(0.0, 1.0, 0.0)) *
        glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

        // neg x
        glm::angleAxis(glm::radians( 90.0), glm::dvec3(0.0, 1.0, 0.0)) *
        glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

        // pos y
        glm::angleAxis(glm::radians(-90.0), glm::dvec3(1.0, 0.0, 0.0)),

        // neg y
        glm::angleAxis(glm::radians( 90.0), glm::dvec3(1.0, 0.0, 0.0)),

        // pos z
        glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 1.0, 0.0)) *
        glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

        // neg z
        glm::angleAxis(glm::radians(  0.0), glm::dvec3(0.0, 1.0, 0.0)) *
        glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),
    };

    const double fov       = M_PI * 0.5;
    const double nearPlane = 0.1;
    const double farPlane  = 150.0;
    projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

    for (size_t face = 0; face < 6; ++face)
        viewMatrices[face] = glm::mat4_cast(rotations[face]);
}

/* ------------------------------------------------------------ *
 * ------------------------------------------------------------ */
glm::dmat4 CubeCamera::cameraMatrix(size_t face) const
{ return projectionMatrix * viewMatrices[face]; }

} // namespace rasperi
} // namespace kuu
