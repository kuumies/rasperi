/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::Transform class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_transform.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dmat4 Transform::matrix() const
{
    glm::dmat4 matrix = glm::mat4(1.0);
    matrix *= glm::mat4_cast(rotation);
    matrix = glm::translate(matrix, position);
    return matrix;
}

} // namespace rasperi
} // namespace kuu
