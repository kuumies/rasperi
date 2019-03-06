/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::ModelImporter class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_model_importer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/geometric.hpp>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include "rasperi_model.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct ModelImporter::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl()
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec3 toVec3(const aiColor3D& c)
    { return glm::dvec3(c.r, c.g, c.b); }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec3 toVec3(const aiVector3D& v)
    { return glm::dvec3(v.x, v.y, v.z); }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::shared_ptr<Mesh> importMesh(const aiMesh* const mesh)
    {
        std::shared_ptr<Mesh> out = std::make_shared<Mesh>();

        for (size_t v = 0; v < mesh->mNumVertices; ++v)
        {
            Vertex vertex;

            if (mesh->HasPositions())
            {
                const aiVector3D* p = &mesh->mVertices[v];
                vertex.position.x = double(p->x);
                vertex.position.y = double(p->y);
                vertex.position.z = double(p->z);
            }

            if (mesh->HasTextureCoords(0))
            {
                const aiVector3D* tc = &mesh->mTextureCoords[0][v];
                vertex.texCoord.x = double(tc->x);
                vertex.texCoord.y = double(tc->y);
            }

            if (mesh->HasNormals())
            {
                const aiVector3D* n = &mesh->mNormals[v];
                vertex.normal.x = double(n->x);
                vertex.normal.y = double(n->y);
                vertex.normal.z = double(n->z);
            }

            if (mesh->HasTangentsAndBitangents())
            {
                const aiVector3D* t = &mesh->mTangents[v];
                vertex.tangent.x = double(t->x);
                vertex.tangent.y = double(t->y);
                vertex.tangent.z = double(t->z);

                const aiVector3D* b = &mesh->mBitangents[v];
                vertex.bitangent.x = double(b->x);
                vertex.bitangent.y = double(b->y);
                vertex.bitangent.z = double(b->z);
            }

            if (mesh->HasVertexColors(0))
            {
                const aiColor4D* c = &mesh->mColors[0][v];
                vertex.color.r = double(c->r);
                vertex.color.g = double(c->g);
                vertex.color.b = double(c->b);
                vertex.color.a = double(c->a);
            }

            out->vertices.push_back(vertex);
        }

        for (size_t i = 0 ; i < mesh->mNumFaces ; i++)
        {
            const aiFace& face = mesh->mFaces[i];
            assert(face.mNumIndices == 3);

            out->indices.push_back(face.mIndices[0]);
            out->indices.push_back(face.mIndices[1]);
            out->indices.push_back(face.mIndices[2]);
        }

        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    QImage loadTexture(const aiMaterial* const material,
                       const aiTextureType textureType,
                       const QDir& dir)
    {
        if (material->GetTextureCount(textureType) <= 0)
            return {};

        aiString path;
        if (material->GetTexture(textureType, 0, &path) != aiReturn_SUCCESS)
            return {};

        QString qpath = QString::fromLatin1(path.C_Str());
        qpath = dir.absoluteFilePath(qpath);
        QImage image(qpath);
        if (image.isNull())
        {
            qDebug() << __FUNCTION__
                     << ": image file is not valid"
                     << qpath;

            return QImage();
        }

        return image;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::shared_ptr<Material> importMaterial(const aiMaterial* const material,
                                             const QDir& dir)
    {
        aiColor3D ambient(0.0f, 0.0f, 0.0f);
        aiColor3D diffuse(1.0f, 1.0f, 1.0f);
        aiColor3D specular(0.0f, 0.0f, 0.0f);
        float specularPower = 1.0f;
        material->Get(AI_MATKEY_COLOR_AMBIENT,  ambient);
        material->Get(AI_MATKEY_COLOR_DIFFUSE,  diffuse);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        material->Get(AI_MATKEY_SHININESS,      specularPower);

        std::shared_ptr<Material> out = std::make_shared<Material>();
        out->phong.ambient       = toVec3(ambient);
        out->phong.diffuse       = toVec3(diffuse);
        out->phong.specular      = toVec3(specular);
        out->phong.specularPower = double(specularPower);

        out->phong.ambientSampler.setMap(loadTexture(material, aiTextureType_AMBIENT, dir));
        out->phong.diffuseSampler.setMap(loadTexture(material, aiTextureType_DIFFUSE, dir));
        out->phong.specularSampler.setMap(loadTexture(material, aiTextureType_SPECULAR, dir));
        out->phong.specularPowerSampler.setMap(loadTexture(material, aiTextureType_SHININESS, dir));
        out->normalSampler.setMap(loadTexture(material, aiTextureType_HEIGHT, dir)); // Note "typo", it really needs to be aiTextureType_HEIGHT for OBJs

        qDebug() << __FUNCTION__
                 << material->GetTextureCount(aiTextureType_AMBIENT)
                 << material->GetTextureCount(aiTextureType_DIFFUSE)
                 << material->GetTextureCount(aiTextureType_SPECULAR)
                 << material->GetTextureCount(aiTextureType_SHININESS)
                 << material->GetTextureCount(aiTextureType_NORMALS)
                 << material->GetTextureCount(aiTextureType_HEIGHT);

        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::shared_ptr<Transform> importTransform(
        const aiString& name,
        const aiScene* const scene)
    {
        std::shared_ptr<Transform> t = std::make_shared<Transform>();
        if (const aiNode* n = scene->mRootNode->FindNode(name))
        {
            glm::dmat4 outMat(1.0);
            {
                aiMatrix4x4 m = n->mTransformation;
                outMat = glm::mat4 {
                    m.a1, m.a2, m.a3, m.a4,
                    m.b1, m.b2, m.b3, m.b4,
                    m.c1, m.c2, m.c3, m.c4,
                    m.d1, m.d2, m.d3, m.d4
                };

                outMat = glm::transpose(outMat);
            }

            glm::dvec3 skew;
            glm::dvec4 perspective;
            if (!glm::decompose(outMat, t->scale, t->rotation, t->position,
                                skew, perspective))
            {
                std::cerr << __FUNCTION__
                          << "Failed to decompose a transform matrix"
                          << std::endl;
            }

            t->rotation = glm::inverse(t->rotation);
            t->position *= 0.01;
            t->scale    *= 0.01;
        }

        return t;
    }
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
ModelImporter::ModelImporter()
    : impl(std::make_shared<Impl>())
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
std::vector<Model> ModelImporter::import(const QString& filepath) const
{
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(
            filepath.toStdString().c_str(),
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_CalcTangentSpace);

    if (!scene)
        return {};

    qDebug() << __FUNCTION__
             << scene->mNumMeshes
             << scene->mNumMaterials
             << scene->mNumTextures;

    const QDir dir = QFileInfo(filepath).absoluteDir();

    std::vector<Model> out;
    for (unsigned int c = 0; c < scene->mRootNode->mNumChildren; ++c)
    {
        aiNode* child = scene->mRootNode->mChildren[c];
        if (!child)
            return {};

        for (unsigned int m = 0; m < child->mNumMeshes; ++m)
        {
            const aiMesh* const mesh =
                scene->mMeshes[child->mMeshes[m]];
            if (!mesh)
                return {};

            qDebug() << __FUNCTION__
                     << c
                     << mesh->mNumVertices
                     << mesh->mNumFaces
                     << mesh->HasPositions()
                     << mesh->HasNormals()
                     << mesh->HasVertexColors(0)
                     << mesh->HasTextureCoords(0)
                     << mesh->HasTangentsAndBitangents();

            Model model;
            model.name = std::string(mesh->mName.C_Str());
            model.mesh = impl->importMesh(mesh);
            model.transform = impl->importTransform(child->mName, scene);
            if (scene->HasMaterials())
                model.material = impl->importMaterial(scene->mMaterials[mesh->mMaterialIndex], dir);
            out.push_back(model);
        }
    }
    return out;
}

} // namespace rasperi
} // namespace kuu
