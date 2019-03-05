/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::Material struct.
 * ---------------------------------------------------------------- */
 
#pragma once

#include "rasperi_sampler.h"
#include <glm/vec3.hpp>
#include "rasperi_texture_cube.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Material
{
    enum class Model
    {
        Phong,
        Pbr
    };

    struct Phong
    {
        bool diffuseFromVertex = false;

        glm::dvec3 ambient   = glm::dvec3(0.05);
        glm::dvec3 diffuse   = glm::dvec3(0.0);
        glm::dvec3 specular  = glm::dvec3(0.3);
        double specularPower = 64.0;
        Sampler ambientSampler;
        Sampler diffuseSampler;
        Sampler specularSampler;
        Sampler specularPowerSampler;
    };

    struct Pbr
    {
        bool albedoFromVertex = false;

        glm::dvec3 albedo  = glm::dvec3(0.05);
        double roughness   = 0.5;
        double metalness   = 0.5;
        double ao          = 1.0;
        Sampler albedoSampler;
        Sampler roughnessSampler;
        Sampler metalnessSampler;
        Sampler aoSampler;

        TextureCube<double, 4>* irradiance = nullptr;
        TextureCube<double, 4>* prefilter = nullptr;
        Texture2D<double, 2>* brdfIntegration = nullptr;
    };

    Model model = Model::Phong;
    Phong phong;
    Pbr pbr;
    Sampler normalSampler;
    Sampler heightSampler;
    Sampler opacitySampler;
};

} // namespace rasperi
} // namespace kuu
