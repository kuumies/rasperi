/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLTriangleMesh class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vector>
#include <glad/glad.h>
#include "rasperi_lib/rasperi_mesh.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLTriangleMesh
{
public:
    OpenGLTriangleMesh(const Mesh& mesh);

    void draw();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
