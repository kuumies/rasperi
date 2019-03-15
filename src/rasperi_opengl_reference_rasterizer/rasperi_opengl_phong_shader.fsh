/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of Phong fragment shader
 * ---------------------------------------------------------------- */

#version 330 core

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
uniform vec3 cameraPosition;
uniform vec3 sunDirection;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float specularPower;
uniform sampler2D ambientSampler;
uniform sampler2D diffuseSampler;
uniform sampler2D specularSampler;
uniform sampler2D specularPowerSampler;
uniform sampler2D normalSampler;
uniform bool useAmbientSampler;
uniform bool useDiffuseSampler;
uniform bool useSpecularSampler;
uniform bool useSpecularPowerSampler;
uniform bool useNormalSampler;
uniform bool rgbSpecularSampler;

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
    // Calculate vectors.
    vec3 n = normalize(vsOut.worldNormal);
    // Use normal form map if available
    if (textureSize(normalSampler, 0).x > 1)
    {
        n = texture(normalSampler, vsOut.texCoord).rgb;
        n = normalize(n * 2.0 - 1.0);
        n = vsOut.tbn * n;
        n = normalize(n);
    }

    vec3 v = normalize(cameraPosition - vsOut.worldPos);
    vec3 l = normalize(-sunDirection.xyz);
    vec3 r = reflect(-l, n);

    float nDotL = dot(n, l);
    nDotL = clamp(nDotL, 0.0, 1.0);

    float vDotR = dot(v, r);
    vDotR = clamp(vDotR, 0.0, 1.0);

    vec3 am = ambient;
    if (useAmbientSampler)
        am = texture(ambientSampler, vsOut.texCoord).rgb;
    vec3 dm = diffuse;
    if (useDiffuseSampler)
        dm = texture(diffuseSampler, vsOut.texCoord).rgb;
    vec3 sm = specular;
    if (useSpecularSampler)
    {
        if (rgbSpecularSampler)
            sm = texture(specularSampler, vsOut.texCoord).rgb;
        else
            sm = vec3(texture(specularSampler, vsOut.texCoord).r);
    }
    float spm = specularPower;
    if (useSpecularPowerSampler)
        spm = texture(specularPowerSampler, vsOut.texCoord).r;

    dm *= nDotL;
    sm = sm * pow(vDotR, spm);

    vec3 c = dm + sm;
    //c = c / (c + vec3(1.0));
    c = pow(c, vec3(1.0 / 2.2));
    color.rgb = c;
    color.a   = 1.0;
}
