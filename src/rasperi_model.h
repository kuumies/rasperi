/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Model class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <string>
#include "rasperi_material.h"
#include "rasperi_mesh.h"
#include "rasperi_transform.h"

namespace kuu
{
namespace rasperi
{

struct Model
{
    std::string name;
    std::shared_ptr<Material> material;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Transform> transform;
};

} // namespace rasperi
} // namespace kuu
