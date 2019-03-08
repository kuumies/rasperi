/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   OpenGL PBR IBL prefilter fragment shader.
 * ---------------------------------------------------------------- */

#version 330 core  
 
/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
const float PI = 3.14159265359;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
uniform samplerCube skyboxMap;
uniform float roughness;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
in vec3 texCoord;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
out vec4 outColor;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
vec2 hammersley(uint i, uint n)
{
    return vec2(float(i)/float(n), radicalInverse_VdC(i));
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
vec3 importanceSampleGGX(vec2 xi, vec3 n, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 h;
    h.x = cos(phi) * sinTheta;
    h.y = sin(phi) * sinTheta;
    h.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0)
                                      : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);

    vec3 sampleVec = tangent * h.x + bitangent * h.y + n * h.z;
    return normalize(sampleVec);
}

/* ---------------------------------------------------------------- *
   Cook-Torrance specular BRDF (GGX) normal distribution function.
 * ---------------------------------------------------------------- */
float brdfNormalDistributionGGX(float nDotH, float a)
{
    float nDotH2 = nDotH * nDotH;
    float a2     = a * a;
    float q      = (nDotH2 * (a2 -1.0) + 1.0);
    return a2 / (PI * q * q);
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void main()
{
    vec3 n = normalize(texCoord);
    vec3 r = n;
    vec3 v = r;

    const uint SAMPLE_COUNT = 2048u * 1u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 xi = hammersley(i, SAMPLE_COUNT);
        vec3 h  = importanceSampleGGX(xi, n, roughness);
        vec3 l  = normalize(2.0 * dot(v, h) * h - v);

        float nDotL = max(dot(n, l), 0.0);
        if(nDotL > 0.0)
        {
            float nDotH = max(dot(n, h), 0.0);
            float hDotV = max(dot(h, v), 0.0);
            float d   = brdfNormalDistributionGGX(nDotH, roughness);
            float pdf = (d * nDotH / (4.0 * hDotV)) + 0.0001;

            // resolution of source cubemap (per face)
            float resolution = 512.0;
            float saTexel  = 4.0 *
                    PI / (6.0 * resolution * resolution);
            float saSample = 1.0 /
                    (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0
                    ? 0.0
                    : 0.5 * log2(saSample / saTexel);

            prefilteredColor +=
                textureLod(skyboxMap, l, mipLevel).rgb * nDotL;
            //prefilteredColor += texture(skyboxMap, l).rgb * nDotL;
            totalWeight      += nDotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    outColor = vec4(prefilteredColor, 1.0);
}
