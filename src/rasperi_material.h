/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of types of kuu::rasperi::Material class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include "rasperi_sampler.h"
#include <glm/vec3.hpp>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Material
{
    bool diffuseFromVertex = false;

    glm::dvec3 ambient   = glm::dvec3(0.1);
    glm::dvec3 diffuse   = glm::dvec3(0.0);
    glm::dvec3 specular  = glm::dvec3(0.3);
    double specularPower = 4.0;

    Sampler ambientSampler;
    Sampler diffuseSampler;
    Sampler specularSampler;
    Sampler specularPowerSampler;
    Sampler normalSampler;
};

} // namespace rasperi
} // namespace kuu
