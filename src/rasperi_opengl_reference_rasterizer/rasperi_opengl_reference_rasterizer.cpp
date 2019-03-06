/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLReferenceRasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_reference_rasterizer.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rasperi_lib/rasperi_material.h"
#include "rasperi_lib/rasperi_mesh.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLFramebuffer
{
public:
    OpenGLFramebuffer(int width, int height)
        : width(width)
        , height(height)
    {}

    int width;
    int height;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLReferenceRasterizer::Impl
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
        //framebuffer.clear();
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
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawEdgeLineTriangleMesh(Mesh* triangleMesh)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void drawLineMesh(Mesh* mesh)
    {}

    OpenGLFramebuffer framebuffer;
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
OpenGLReferenceRasterizer::OpenGLReferenceRasterizer(int width, int height)
    : impl(std::make_shared<Impl>(width, height))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::clear()
{ impl->clear(); }

void OpenGLReferenceRasterizer::setModelMatrix(const glm::dmat4& model)
{
    impl->modelMatrix = model;
    impl->updateMatrices();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::setViewMatrix(const glm::dmat4& view)
{
    impl->viewMatrix = view;
    impl->updateMatrices();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::setProjectionMatrix(const glm::dmat4& projection)
{
    impl->projectionMatrix = projection;
    impl->updateMatrices();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::setMaterial(const Material& material)
{ impl->material = material; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::setNormalMode(OpenGLReferenceRasterizer::NormalMode normalMode)
{ impl->normalMode = normalMode; }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::drawFilledTriangleMesh(Mesh* mesh)
{ impl->drawFilledTriangleMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::drawEdgeLineTriangleMesh(Mesh* mesh)
{ impl->drawEdgeLineTriangleMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::drawLineMesh(Mesh* mesh)
{ impl->drawLineMesh(mesh); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::run(GLuint fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, impl->framebuffer.width, impl->framebuffer.height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLFramebuffer& OpenGLReferenceRasterizer::framebuffer() const
{ return impl->framebuffer; }

} // namespace rasperi
} // namespace kuu
