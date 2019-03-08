/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of OpenGL NDC meshes.
 * ---------------------------------------------------------------- */

#include "rasperi_opengl_ndc_mesh.h"
#include <math.h>
#include <vector>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

/* ---------------------------------------------------------------- *
   NdcQuadMesh
 * ---------------------------------------------------------------- */

NdcQuadMesh::NdcQuadMesh(bool flipY)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    std::vector<float> vertexData =
    {
        -1, -1,  0, 0,
         1, -1,  1, 0,
         1,  1,  1, 1,
        -1,  1,  0, 1,
    };

    if (flipY)
    {
        vertexData[1]  *= -1;
        vertexData[5]  *= -1;
        vertexData[9]  *= -1;
        vertexData[13] *= -1;
    }

    std::vector<unsigned> indexData =
    {
        0, 1, 2,  2, 3, 0
    };
    indexCount = GLuint(indexData.size());

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(float),
                 vertexData.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indexData.size() * sizeof(unsigned),
                 indexData.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float),
                          BUFFER_OFFSET(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float),
                          BUFFER_OFFSET(2 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

NdcQuadMesh::~NdcQuadMesh()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

void NdcQuadMesh::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/* ---------------------------------------------------------------- *
   NdcCubeMesh
 * ---------------------------------------------------------------- */

NdcCubeMesh::NdcCubeMesh(bool windingCCW)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    std::vector<float> vertexData =
    {
        -1,-1,-1,
         1, 1,-1,
         1,-1,-1,
        -1, 1,-1,
        -1,-1, 1,
         1,-1, 1,
         1, 1, 1,
        -1, 1, 1,
    };

    std::vector<unsigned> indexData;
    if (windingCCW)
    {
        indexData =
        {
            0,1,2, 1,0,3,
            4,5,6, 6,7,4,
            7,3,0, 0,4,7,
            6,2,1, 2,6,5,
            0,2,5, 5,4,0,
            3,6,1, 6,3,7,
        };
    }
    else
    {
        indexData =
        {
            2,1,0, 3,0,1,
            6,5,4, 4,7,6,
            0,3,7, 7,4,0,
            1,2,6, 5,6,2,
            5,2,0, 0,4,5,
            1,6,3, 7,3,6,
        };
    }

    indexCount = GLuint(indexData.size());

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(float),
                 vertexData.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indexData.size() * sizeof(unsigned int),
                 indexData.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), BUFFER_OFFSET(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

NdcCubeMesh::~NdcCubeMesh()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

void NdcCubeMesh::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/* ---------------------------------------------------------------- *
   NdcSphereMesh
 * ---------------------------------------------------------------- */

NdcSphereMesh::NdcSphereMesh(bool windingClockWise)
{
    using namespace glm;

    const int ringCount   = 16;
    const int sectorCount = 16;
    //const float radius    = 1.0f;
    const float radius    = 642.0f;
    const float pi        = float(M_PI);
    const float half_pi   = pi * 0.5f;

    const float ringStep   = 1.0f / float(ringCount   - 1);
    const float sectorStep = 1.0f / float(sectorCount - 1);

    std::vector<float> vertexData;
    for(int r = 0; r < ringCount;   ++r)
    for(int s = 0; s < sectorCount; ++s)
    {
        float y = sin(half_pi + pi * r * ringStep);
        float x = cos(2.0f * pi * s * sectorStep) * sin(pi * r * ringStep);
        float z = sin(2.0f * pi * s * sectorStep) * sin(pi * r * ringStep);

        vec3 pos = vec3(x, y, z) * radius;
        vec2 uv(s * sectorStep, r * ringStep);

        vertexData.push_back(pos.x);
        vertexData.push_back(pos.y);
        vertexData.push_back(pos.z);
        vertexData.push_back(uv.x);
        vertexData.push_back(uv.y);
    }

    std::vector<unsigned int> indexData;
    for(int r = 0; r < ringCount   - 1; r++)
    for(int s = 0; s < sectorCount - 1; s++)
    {
        int ia = (r+0) * sectorCount + (s+0);
        int ib = (r+0) * sectorCount + (s+1);
        int ic = (r+1) * sectorCount + (s+1);
        int id = (r+1) * sectorCount + (s+0);

        if (windingClockWise)
        {
            indexData.push_back(ib);
            indexData.push_back(ia);
            indexData.push_back(id);

            indexData.push_back(id);
            indexData.push_back(ic);
            indexData.push_back(ib);
        }
        else
        {
            indexData.push_back(id);
            indexData.push_back(ia);
            indexData.push_back(ib);

            indexData.push_back(ib);
            indexData.push_back(ic);
            indexData.push_back(id);
        }
    }

    indexCount = GLuint(indexData.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(float),
                 vertexData.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indexData.size() * sizeof(unsigned int),
                 indexData.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float),
                          BUFFER_OFFSET(0));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float),
                          BUFFER_OFFSET(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

NdcSphereMesh::~NdcSphereMesh()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

void NdcSphereMesh::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace rasperi
} // namespace kuu
