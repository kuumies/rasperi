/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::ModelImporter class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <QtCore/QString>
#include "rasperi_mesh.h"
#include "rasperi_material.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class ModelImporter
{
public:
    struct Transform
    {
        glm::dvec3 position;
        glm::dquat rotation;
        glm::dvec3 scale = glm::dvec3(1.0);
    };

    struct Model
    {
        std::shared_ptr<Material> material;
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Transform> transform;
    };

    ModelImporter();
    std::vector<Model> import(const QString& filepath) const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
