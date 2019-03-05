/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::ModelImporter class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <QtCore/QString>
#include "rasperi_model.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class ModelImporter
{
public:
    ModelImporter();
    std::vector<Model> import(const QString& filepath) const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
