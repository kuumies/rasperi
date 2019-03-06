/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::Rasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_rasterizer.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rasperi_material.h"
#include "rasperi_mesh.h"
#include "rasperi_primitive_rasterizer.h"
#include "rasperi_sampler.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Rasterizer::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(int width, int height)
        : framebuffer(width, height)
        , normalMode(NormalMode::Coarse)
    {
        viewMatrix = glm::translate(glm::dmat4(1.0), glm::dvec3(0, 0, 3.0));
        projectionMatrix = glm::perspective(M_PI * 0.25, width / double(height), 0.1, 150.0);
        lightDir = glm::dvec3(0, 0, -1);
        material.phong.diffuse = glm::dvec3(1.0);
        material.phong.diffuseFromVertex = false;
        updateMatrices();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void clear()
    {
        framebuffer.clear();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void updateMatrices()
    {
        const glm::dmat4 viewInvMatrix = glm::inverse(viewMatrix);
        cameraPos = glm::dvec3(viewInvMatrix * glm::dvec4(0.0, 0.0, 0.0, 1.0));
        cameraMatrix = projectionMatrix * viewMatrix * modelMatrix;
        normalMatrix = glm::inverseTranspose(glm::dmat3(modelMatrix));
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawFilledTriangleMesh(Mesh* mesh)
    {
        TrianglePrimitiveRasterizer triRast(framebuffer, normalMode);
        triRast.rasterize(*mesh, cameraMatrix, modelMatrix, normalMatrix, lightDir, cameraPos, material);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawEdgeLineTriangleMesh(Mesh* triangleMesh)
    {
        unsigned index = 0;
        Mesh lineMesh;
        for (size_t i = 0; i < triangleMesh->indices.size(); i += 3)
        {
            unsigned i1 = triangleMesh->indices[i + 0];
            unsigned i2 = triangleMesh->indices[i + 1];
            unsigned i3 = triangleMesh->indices[i + 2];
            if (i1 >= triangleMesh->vertices.size() ||
                i2 >= triangleMesh->vertices.size() ||
                i3 >= triangleMesh->vertices.size())
            {
                std::cout << "invalid index"
                          << i1 << ", " << i2 << ", " << i3 << " ? "
                          << triangleMesh->vertices.size()
                          << std::endl;
                continue;
            }

            Triangle tri;
            tri.p1 = triangleMesh->vertices[i1];
            tri.p2 = triangleMesh->vertices[i2];
            tri.p3 = triangleMesh->vertices[i3];
            tri.p1.color = glm::dvec4(1.0);
            tri.p2.color = glm::dvec4(1.0);
            tri.p3.color = glm::dvec4(1.0);

            lineMesh.vertices.push_back(tri.p1);
            lineMesh.vertices.push_back(tri.p2);
            lineMesh.vertices.push_back(tri.p3);


            i1 = index++;
            i2 = index++;
            i3 = index++;

            lineMesh.indices.push_back(i1);
            lineMesh.indices.push_back(i2);
            lineMesh.indices.push_back(i2);
            lineMesh.indices.push_back(i3);
            lineMesh.indices.push_back(i3);
            lineMesh.indices.push_back(i1);
        }

        LinePrimitiveRasterizer linRast(framebuffer);
        linRast.rasterize(lineMesh, cameraMatrix);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawLineMesh(Mesh* mesh)
    {
        LinePrimitiveRasterizer linRast(framebuffer);
        linRast.rasterize(*mesh, cameraMatrix);
    }

    Framebuffer framebuffer;
    NormalMode normalMode;
    glm::dmat4 modelMatrix;
    glm::dmat4 viewMatrix;
    glm::dmat4 projectionMatrix;
    glm::dmat4 cameraMatrix;
    glm::dmat3 normalMatrix;
    glm::dvec3 lightDir;
    glm::dvec3 cameraPos;
    Material material;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Rasterizer::Rasterizer(int width, int height)
    : impl(std::make_shared<Impl>(width, height))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::clear()
{ impl->clear(); }

void Rasterizer::setModelMatrix(const glm::dmat4& model)
{
    impl->modelMatrix = model;
    impl->updateMatrices();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setViewMatrix(const glm::dmat4& view)
{
    impl->viewMatrix = view;
    impl->updateMatrices();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setProjectionMatrix(const glm::dmat4& projection)
{
    impl->projectionMatrix = projection;
    impl->updateMatrices();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setMaterial(const Material& material)
{ impl->material = material; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setNormalMode(Rasterizer::NormalMode normalMode)
{ impl->normalMode = normalMode; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::drawFilledTriangleMesh(Mesh* mesh)
{ impl->drawFilledTriangleMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::drawEdgeLineTriangleMesh(Mesh* mesh)
{ impl->drawEdgeLineTriangleMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::drawLineMesh(Mesh* mesh)
{ impl->drawLineMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
Framebuffer &Rasterizer::framebuffer() const
{ return impl->framebuffer; }

} // namespace rasperi
} // namespace kuu
