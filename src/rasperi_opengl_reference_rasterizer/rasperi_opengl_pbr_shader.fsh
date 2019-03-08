/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   PBR fragment shader
 * ---------------------------------------------------------------- */

#version 330 core

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
const float PI = 3.14159;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
uniform vec3 cameraPosition;
uniform vec3 sunDirection;
uniform vec3 albedoValue;
uniform float roughnessValue;
uniform float metalnessValue;
uniform float aoValue;
uniform sampler2D albedoSampler;
uniform sampler2D roughnessSampler;
uniform sampler2D metalnessSampler;
uniform sampler2D aoSampler;
uniform sampler2D normalSampler;
uniform bool useAlbedoSampler;
uniform bool useRoughnessSampler;
uniform bool useMetalnessSampler;
uniform bool useAoSampler;
uniform bool useNormalSampler;
uniform samplerCube irradianceSampler;
uniform samplerCube prefilterSampler;
uniform sampler2D brdfIntegrationSampler;
uniform int prefilterSamplerMipmapCount;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
in struct VsOut
{
    vec2 texCoord;
    vec3 worldNormal;
    vec3 worldPos;
    mat3 tbn;
} vsOut;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
out vec4 color;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void main()
{
    // ------------------------------------------------------------
    // Calculate vectors
    
    vec3 n = normalize(vsOut.worldNormal);
    if (useNormalSampler)
    {
        n = texture(normalSampler, vsOut.texCoord).rgb;
        n = normalize(n * 2.0 - 1.0);
        n = vsOut.tbn * n;
        n = normalize(n);
    }

    vec3 v = normalize(cameraPosition - vsOut.worldPos);
    vec3 l = normalize(-sunDirection.xyz);
    vec3 h = normalize(l + v);
    vec3 r = reflect(-v, n);
    
    // ------------------------------------------------------------
    // Calculate angles
    
    float nDotL = clamp(dot(n, l), 0.0, 1.0);
    float nDotV = clamp(dot(n, v), 0.0, 1.0);
    float nDotH = clamp(dot(n, h), 0.0, 1.0);

    // ------------------------------------------------------------
    // Collect material
    
    vec3 albedo = albedoValue;
    if (useAlbedoSampler)
        albedo = texture(albedoSampler, vsOut.texCoord).rgb;
        
    float roughness = roughnessValue;
    if (useRoughnessSampler)
        roughness = texture(roughnessSampler, vsOut.texCoord).r;
        
    float metalness = metalnessValue;
    if (useMetalnessSampler)
        metalness = texture(metalnessSampler, vsOut.texCoord).r;
    
    float ao = aoValue;
    if (useAoSampler)
        ao = texture(aoSampler, vsOut.texCoord).r;

    // ------------------------------------------------------------
    // Radiance specular
        
    // Base reflectivity
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metalness);

    // GGX normal distribution function
    float ndf = 0.0;
    {
        float nDotH2 = nDotH * nDotH;
        float a  = roughness * roughness;
        float a2 = a * a;
        float q  = nDotH2 * (a2 - 1.0) + 1.0;
        ndf = a2 / (PI * q * q);
    }

    // GGX geometry function
    float g = 0.0f;
    {
        float k = roughness;
        float ggx1 = nDotV / (nDotV * (1.0 - k) + k);
        float ggx2 = nDotL / (nDotL * (1.0 - k) + k);
        g = ggx1 * ggx2;
    }

    // Fresnel
    vec3 f = f0 + (1.0 - f0) * pow(1.0 - nDotV, 5.0); // is this nDotL?
    
    vec3 radianceSpecular = (ndf * g * f ) / max(4.0 * nDotV * nDotL, 0.001);
            
    // ------------------------------------------------------------
    // Radiance diffuse
    
    // Reflection/refraction ratio
    vec3 kS = f;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    // Calc. light diffuse and specular radiance
    vec3 radianceDiffuse  = kD * albedo / PI;

    // ------------------------------------------------------------
    // Radiance 
    
    vec3 sunIntensity = vec3(1.0);
    vec3 radiance = (radianceDiffuse + radianceSpecular) * sunIntensity * nDotL;
    
    // ------------------------------------------------------------
    // Irradiance diffuse

    vec3 irradianceDiffuse  = texture(irradianceSampler, n).rgb * albedo;

    // ------------------------------------------------------------
    // Irradiance specular
    
    //vec3 prefiltered = textureLod(prefilterSampler, r, roughness * float(prefilterSamplerMipmapCount)).rgb;
    vec3 prefiltered = texture(prefilterSampler, r).rgb;
    
    
    vec2 brdf  = texture(brdfIntegrationSampler, vec2(nDotV, roughness)).rg;
//    // Fresnel roughness
    vec3 fr = f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - nDotV, 5.0);
    
    vec3 irradianceSpecular = prefiltered * (fr * brdf.x + brdf.y);

    // ------------------------------------------------------------
    // Irradiance

    vec3 irradiance = (kD * irradianceDiffuse + irradianceSpecular) * ao;

    // ------------------------------------------------------------
    // Post-process 
    
    vec3 c = radiance /*+ irradiance*/;
    c = c / (c + vec3(1.0));
    c = pow(c, vec3(1.0 / 2.2));
    
    color.rgb = c;
    color.a   = 1.0;
}
