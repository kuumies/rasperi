/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::ModelImporter class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vector>

class QString;

namespace kuu
{
namespace rasperi
{

class Model;

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
