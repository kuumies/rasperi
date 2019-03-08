/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of OpenGL NDC meshes.
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>
#include <glad/glad.h>
#include <glm/vec2.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
   NDC quad mesh.

   * mesh has a two vertex attributes:
        1) position
        2) texture coordinate.
 * ---------------------------------------------------------------- */
struct NdcQuadMesh
{
    NdcQuadMesh(bool flipY  = false);
    ~NdcQuadMesh();

    void draw();

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint indexCount;
};

/* ---------------------------------------------------------------- *
   NDC cube mesh.

   * winding can be chosen to avoid different cull modes.
   * mesh has a single vertex attribute: position
 * ---------------------------------------------------------------- */
struct NdcCubeMesh
{
    NdcCubeMesh(bool windingCCW = false);
    ~NdcCubeMesh();

    void draw();

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint indexCount;
};

/* ---------------------------------------------------------------- *
   NDC sphere mesh.

   * winding can be chosen to avoid different cull modes.
   * mesh has a two vertex attributes:
        1) position
        2) texture coordinate.
 * ---------------------------------------------------------------- */
struct NdcSphereMesh
{
    NdcSphereMesh(bool windingClockWise = true);
    ~NdcSphereMesh();

    void draw();

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint indexCount;
};

} // namespace rasperi
} // namespace kuu
