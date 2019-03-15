/* -----------------------------------------------------------------*
   Antti Jumpponen <kuumies@gmail.com>
   The impelmentation of kuu::Frustum struct.
 * -----------------------------------------------------------------*/

#include "frustum.h"
#include "projector.h"

namespace kuu
{

Frustum::Frustum(const glm::mat4 &camera,
                 const glm::vec4& viewport)
{
    float w = viewport.z;
    float h = viewport.w;

    const Projector projector(camera, viewport);

    corners.push_back(projector.unproject(0.0f, 0.0f, 0.0f));
    corners.push_back(projector.unproject(w,    0.0f, 0.0f));
    corners.push_back(projector.unproject(0.0f, h,    0.0f));
    corners.push_back(projector.unproject(w,    h,    0.0f));
    corners.push_back(projector.unproject(0.0f, 0.0f, 1.0f));
    corners.push_back(projector.unproject(w,    0.0f, 1.0f));
    corners.push_back(projector.unproject(0.0f, h,    1.0f));
    corners.push_back(projector.unproject(w,    h,    1.0f));
}

glm::vec3 Frustum::centroid() const
{
    glm::vec3 c;

    for (size_t i = 0; i < 8; i++)
        c += corners[i];

    return c * 1.0f / 8.0f;
}

glm::vec3 Frustum::farCenter() const
{
    glm::vec3 c;
    for (size_t i = 4; i < 8; i++)
        c += corners[i];

    return c * 1.0f / 4.0f;
}

glm::vec3 Frustum::nearCenter() const
{
    glm::vec3 c;
    for (size_t i = 0; i < 4; i++)
        c += corners[i];

    return c * 1.0f / 4.0f;
}

} // namespace kuu

