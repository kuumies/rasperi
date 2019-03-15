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
#include "rasperi_opengl_background.h"
#include "rasperi_opengl_equirectangular_to_cubemap.h"
#include "rasperi_opengl_pbr_ibl.h"
#include "rasperi_opengl_ndc_mesh.h"
#include "rasperi_opengl_pbr_shader.h"
#include "rasperi_opengl_pbr_textures.h"
#include "rasperi_opengl_phong_shader.h"
#include "rasperi_opengl_phong_textures.h"
#include "rasperi_opengl_triangle_mesh.h"
#include "rasperi_opengl_sky_box_shader.h"

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
        : pbrShader(new OpenGLPbrShader())
        , phongShader(new OpenGLPhongShader())
        , skyBoxShader(new OpenGLSkyBoxShader())
        , bg(nullptr)
        , eToCm(512)
    {}

    ~Impl()
    {
        delete skyBoxShader;
        delete bg;
        delete phongShader;
        for (auto& meshes : triangleMeshes)
            delete meshes.second;
        for (auto& texture : phongTextures)
            delete texture.second;
        for (auto& texture : pbrTextures)
            delete texture.second;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(GLuint fbo, const Scene& scene)
    {
#if 0
        if (!bg)
        {
            std::shared_ptr<NdcQuadMesh> ndcQuad = std::make_shared<NdcQuadMesh>();
            std::shared_ptr<NdcCubeMesh> ndcCube = std::make_shared<NdcCubeMesh>();

            eToCm.run(scene.skyTexture);

            bg = new OpenGLBackground(scene.background);
            pbrIbl = renderPbrImageLighting(ndcQuad,
                                            ndcCube,
                                            glm::ivec2(512, 512),
                                            glm::ivec2(512, 512),
                                            glm::ivec2(512, 512),
                                            eToCm.cubemap);
        }
#endif

        const glm::dmat4 viewInvMatrix = glm::inverse(scene.view);
        const glm::dvec3 cameraPos     = glm::dvec3(viewInvMatrix * glm::dvec4(0.0, 0.0, 0.0, 1.0));
        const glm::dvec3 lightDir      = glm::dvec3(0, 0, -1);

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, scene.viewport.z, scene.viewport.w);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
        //glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, eToCm.cubemap);
        skyBoxShader->viewMatrix       = scene.view;
        skyBoxShader->projectionMatrix = scene.projection;
        skyBoxShader->use();
        ndcCube.draw();
        glUseProgram(0);
        glDepthFunc(GL_LESS);
        //glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
#endif

        for (const Model& model : scene.models)
        {
            switch (model.material->model)
            {
                case Material::Model::Phong:
                {
                    OpenGLTriangleMesh* triMesh = nullptr;
                    for (auto& meshes : triangleMeshes)
                        if (meshes.first == model.mesh.get())
                            triMesh = meshes.second;
                    if (!triMesh)
                    {
                        triMesh = new OpenGLTriangleMesh(*model.mesh);
                        triangleMeshes[model.mesh.get()] = triMesh;
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
                    else
                        phongShader->modelMatrix         = glm::mat4(1.0);
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
                    phongShader->rgbSpecularSampler      = !model.material->phong.specularSampler.map().isGrayscale();
                    phongShader->use();
                    phongTexture->bind();
                    triMesh->draw();
                    break;
                }
                
                case Material::Model::Pbr:
                {
                    OpenGLTriangleMesh* triMesh = nullptr;
                    for (auto& meshes : triangleMeshes)
                        if (meshes.first == model.mesh.get())
                            triMesh = meshes.second;
                    if (!triMesh)
                    {
                        triMesh = new OpenGLTriangleMesh(*model.mesh);
                        triangleMeshes[model.mesh.get()] = triMesh;
                    }

                    OpenGLPbrTexture* pbrTexture = nullptr;
                    for (auto& texture : pbrTextures)
                        if (texture.first == model.material.get())
                            pbrTexture = texture.second;
                    if (!pbrTexture)
                    {
                        pbrTexture = new OpenGLPbrTexture(*model.material);
                        pbrTextures[model.material.get()] = pbrTexture;
                    }

                    if (model.transform)
                        pbrShader->modelMatrix             = model.transform->matrix();
                    else
                        pbrShader->modelMatrix             = glm::mat4(1.0);
                    pbrShader->viewMatrix                  = scene.view;
                    pbrShader->projectionMatrix            = scene.projection;
                    pbrShader->lightDirection              = lightDir;
                    pbrShader->cameraPosition              = cameraPos;
                    pbrShader->albedo                      = model.material->pbr.albedo;
                    pbrShader->roughness                   = model.material->pbr.roughness;
                    pbrShader->metalness                   = model.material->pbr.metalness;
                    pbrShader->ao                          = model.material->pbr.ao;
                    pbrShader->useAlbedoSampler            = model.material->pbr.albedoSampler.isValid();
                    pbrShader->useRoughnessSampler         = model.material->pbr.roughnessSampler.isValid();
                    pbrShader->useMetalnessSampler         = model.material->pbr.metalnessSampler.isValid();
                    pbrShader->useAoSampler                = model.material->pbr.aoSampler.isValid();
                    pbrShader->useNormalSampler            = model.material->normalSampler.isValid();
                    //pbrShader->prefilterSamplerMipmapCount = pbrIbl->prefilterMipmapCount;
                    pbrShader->use();
                    pbrTexture->bind();
                    //pbrIbl->bind();
                    triMesh->draw();
                    break;
                }
            }
        }
    }

    std::map<Mesh*, OpenGLTriangleMesh*> triangleMeshes;
    std::map<Material*, OpenGLPbrTexture*> pbrTextures;
    std::map<Material*, OpenGLPhongTexture*> phongTextures;
    OpenGLPbrShader* pbrShader;
    OpenGLPhongShader* phongShader;
    OpenGLSkyBoxShader* skyBoxShader;
    OpenGLBackground* bg;
    std::shared_ptr<PbrImageLightingTextures> pbrIbl;
    NdcCubeMesh ndcCube;
    OpenGLEquirectangularToCubemap eToCm;
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
