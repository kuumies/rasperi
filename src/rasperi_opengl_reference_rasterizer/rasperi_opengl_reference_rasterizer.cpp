/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLReferenceRasterizer class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_reference_rasterizer.h"
#include <map>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rasperi_lib/rasperi_material.h"
#include "rasperi_lib/rasperi_mesh.h"
#include "rasperi_opengl_phong_mesh.h"
#include "rasperi_opengl_phong_shader.h"
#include "rasperi_opengl_phong_textures.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLReferenceRasterizer::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl()
        : phongShader(new OpenGLPhongShader())
    {}

    ~Impl()
    {
        delete phongShader;
        for (auto& meshes : phongMeshes)
            delete meshes.second;
        for (auto& texture : phongTextures)
            delete texture.second;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(GLuint fbo, const Scene& scene)
    {
        const glm::dmat4 viewInvMatrix = glm::inverse(scene.view);
        const glm::dvec3 cameraPos     = glm::dvec3(viewInvMatrix * glm::dvec4(0.0, 0.0, 0.0, 1.0));
        const glm::dvec3 lightDir      = glm::dvec3(0, 0, -1);

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, scene.viewport.z, scene.viewport.w);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const Model& model : scene.models)
        {
            if (model.material->model == Material::Model::Phong)
            {
                OpenGLPhongMesh* phongMesh = nullptr;
                for (auto& meshes : phongMeshes)
                    if (meshes.first == model.mesh.get())
                        phongMesh = meshes.second;
                if (!phongMesh)
                {
                    phongMesh = new OpenGLPhongMesh(*model.mesh);
                    phongMeshes[model.mesh.get()] = phongMesh;
                }

                OpenGLPhongTexture* phongTexture = nullptr;
                for (auto& texture : phongTextures)
                    if (texture.first == model.material.get())
                        phongTexture = texture.second;
                if (!phongTexture)
                {
                    phongTexture = new OpenGLPhongTexture(*model.material);
                    phongTextures[model.material.get()] = phongTexture;
                }

                if (model.transform)
                    phongShader->modelMatrix         = model.transform->matrix();
                phongShader->viewMatrix              = scene.view;
                phongShader->projectionMatrix        = scene.projection;
                phongShader->lightDirection          = lightDir;
                phongShader->cameraPosition          = cameraPos;
                phongShader->ambient                 = model.material->phong.ambient;
                phongShader->diffuse                 = model.material->phong.diffuse;
                phongShader->specular                = model.material->phong.specular;
                phongShader->specularPower           = model.material->phong.specularPower;
                phongShader->useAmbientSampler       = model.material->phong.ambientSampler.isValid();
                phongShader->useDiffuseSampler       = model.material->phong.diffuseSampler.isValid();
                phongShader->useSpecularSampler      = model.material->phong.specularSampler.isValid();
                phongShader->useSpecularPowerSampler = model.material->phong.specularPowerSampler.isValid();
                phongShader->useNormalSampler        = model.material->normalSampler.isValid();
                phongShader->use();
                phongTexture->bind();
                phongMesh->draw();
            }
        }
    }

    std::map<Mesh*, OpenGLPhongMesh*> phongMeshes;
    std::map<Material*, OpenGLPhongTexture*> phongTextures;
    OpenGLPhongShader* phongShader;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLReferenceRasterizer::OpenGLReferenceRasterizer()
    : impl(std::make_shared<Impl>())
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLReferenceRasterizer::run(GLuint fbo, const Scene& scene)
{ impl->run(fbo, scene); }

} // namespace rasperi
} // namespace kuu
