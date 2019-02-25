/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::Rasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_rasterizer.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rasperi_sampler.h"
#include "rasperi_primitive_rasterizer.h"

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
        : colorFramebuffer(width, height, 4)
        , depthFramebuffer(width, height, 1)
        , normalMode(NormalMode::Coarse)
    {
        view = glm::translate(glm::dmat4(1.0), glm::dvec3(0, 0, 3.0));
        projection = glm::perspective(M_PI * 0.25, width / double(height), 0.1, 150.0);
        lightDir = glm::dvec3(0, 0, -1);
        material.diffuse = glm::dvec3(1.0);
        material.diffuseFromVertex = false;
        updateMatrices();
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void clear()
    {
        colorFramebuffer.set(0);
        depthFramebuffer.set(std::numeric_limits<double>::max());
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void updateMatrices()
    {
        const glm::dmat4 viewInv = glm::inverse(view);
        cameraPos = glm::dvec3(viewInv * glm::dvec4(0.0, 0.0, 0.0, 1.0));
        cameraMatrix = projection * viewInv;
        normalMatrix = glm::inverseTranspose(glm::dmat3(viewInv));
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawTriangleMesh(Mesh* mesh)
    {
        TrianglePrimitiveRasterizer triRast(colorFramebuffer, depthFramebuffer, normalMode);
        triRast.rasterize(*mesh, cameraMatrix, normalMatrix, lightDir, cameraPos, material);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawLineMesh(Mesh* mesh)
    {
        LinePrimitiveRasterizer linRast(colorFramebuffer, depthFramebuffer);
        linRast.rasterize(*mesh, cameraMatrix);
    }

    ColorFramebuffer colorFramebuffer;
    DepthFramebuffer depthFramebuffer;
    NormalMode normalMode;
    glm::dmat4 view;
    glm::dmat4 projection;
    glm::dmat4 cameraMatrix;
    glm::dmat3 normalMatrix;
    std::vector<Mesh*> meshes;
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

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setViewMatrix(const glm::dmat4& view)
{
    impl->view = view;
    impl->updateMatrices();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::setProjectionMatrix(const glm::dmat4& projection)
{
    impl->projection = projection;
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
void Rasterizer::drawTriangleMesh(Mesh* mesh)
{ impl->drawTriangleMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void Rasterizer::drawLineMesh(Mesh* mesh)
{ impl->drawLineMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ColorFramebuffer Rasterizer::colorFramebuffer() const
{ return impl->colorFramebuffer; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
DepthFramebuffer Rasterizer::depthFramebuffer() const
{ return impl->depthFramebuffer; }


} // namespace rasperi
} // namespace kuu
