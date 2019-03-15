/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Projector class.
 * -----------------------------------------------------------------*/

#include "projector.h"
#include <iostream>

namespace kuu
{

Projector::Projector(const glm::mat4 &camera,
                     const glm::vec4& viewport)
    : camera_(camera)
    , viewport(viewport)
{}

glm::vec3 Projector::project(const glm::vec3& p) const
{
    const glm::mat4 camMat = camera_;
    if (camMat == glm::mat4(1.0f))
    {
        std::cerr << "Camera matrix is an identity matrix.";
        return glm::vec3();
    }

    glm::vec4 v = camMat * glm::vec4(p.x, p.y, p.z, 1.0f);

    if (v.w == 0.0f)
    {
        std::cerr << __FUNCTION__
                  << ": failed to transform point to clip space."
                  << std::endl;
        return glm::vec3();
    }

    glm::vec3 window(v);
    window /= v.w;

    if (v.z < 0.0f)
    {
        std::cerr << __FUNCTION__
                  << ": persepective divide failed."
                  << std::endl;
        return glm::vec3();
    }

    const glm::vec4 vp = viewport;
    float x = vp.x + (vp.z * (window.x + 1.0f)) / 2.0f;
    float y = vp.y + (vp.w * (window.y + 1.0f)) / 2.0f;
    float z =                (window.z + 1.0f)  / 2.0f;
    //std::cout << z << std::endl;

    return glm::vec3(x, vp.w - y, z);
}

glm::vec3 Projector::project(float x, float y, float z) const
{
    return project(glm::vec3(x, y, z));
}

glm::vec3 Projector::unproject(const glm::vec3& point,
                               bool topDown) const
{
    const glm::vec4 vp = viewport;

    glm::vec3 originalPoint(point.x, point.y, point.z);

    // Flip y-axis if the point is down-top
    if (!topDown)
        originalPoint = glm::vec3(originalPoint.x,
                                  vp.w - originalPoint.y,
                                  originalPoint.z);

    const glm::mat4 camMat = camera_;
    if (camMat == glm::mat4(1.0f))
    {
        std::cerr << __FUNCTION__
                  << ": camera matrix is an identity matrix."
                  << std::endl;
        return glm::vec3();
    }
    glm::mat4 invCameraMatrix = glm::inverse(camMat);
    if (invCameraMatrix == glm::mat4(1.0f))
    {
        std::cerr << __FUNCTION__
                  << ": failed to find inverse of camera matrix."
                  << std::endl;
        return glm::vec3();
    }

    // Map x and y from window coordinates
    float a = (originalPoint.x - vp.x) / vp.z;
    float b = (originalPoint.y - vp.y) / vp.w;
    float c = originalPoint.z;

    // Map to range -1 to 1
    a = a * 2.0f - 1.0f;
    b = b * 2.0f - 1.0f;
    c = c * 2.0f - 1.0f;

    glm::vec4 p = invCameraMatrix * glm::vec4(a, b, c, 1.0f);

    if (p.w == 0.0f)
    {
        std::cerr << "Inverse projection failed." << std::endl;
        return glm::vec3();
    }
    return glm::vec3(p.x, p.y, p.z) / p.w;
}

glm::vec3 Projector::unproject(float x, float y, float z,
                               bool topDown) const
{
    return unproject(glm::vec3(x, y, z), topDown);
}

Ray Projector::viewportRay(const glm::vec2& pos) const
{
    glm::vec3 nearPlane(pos.x, pos.y, 0.0);
    nearPlane = unproject(nearPlane, false);

    glm::vec3 farPlane(pos.x, pos.y, 1.0);
    farPlane = unproject(farPlane, false);

    return Ray(nearPlane, farPlane - nearPlane);
}

} // namespace kuu
