/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::texture_cube_mapping namspace.
 * ---------------------------------------------------------------- */
 
#include "rasperi_texture_cube_mapping.h"
#include <iostream>
#include <glm/common.hpp>

namespace kuu
{
namespace rasperi
{
namespace texture_cube_mapping
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
glm::dvec3 mapTextureCoordinate(TextureCoordinate tc)
{
    // convert range 0 to 1 to -1 to 1
    double uc = 2.0 * tc.uv.x - 1.0;
    double vc = 2.0 * tc.uv.y - 1.0;

    glm::dvec3 out;
    switch (tc.faceIndex)
    {
        case 0: out.x =  1.0; out.y =   vc; out.z =  -uc; break;	// POSITIVE X
        case 1: out.x = -1.0; out.y =   vc; out.z =   uc; break;	// NEGATIVE X
        case 2: out.x =   uc; out.y =  1.0; out.z =  -vc; break;	// POSITIVE Y
        case 3: out.x =   uc; out.y = -1.0; out.z =   vc; break;	// NEGATIVE Y
        case 4: out.x =   uc; out.y =   vc; out.z =  1.0; break;	// POSITIVE Z
        case 5: out.x =  -uc; out.y =   vc; out.z = -1.0; break;	// NEGATIVE Z
    }
    return out;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
TextureCoordinate doIt(const glm::dvec3& p)
{
    TextureCoordinate out;

    // Find the largest magnitude
    glm::dvec3 ap = glm::abs(p);
    if (ap.x > ap.y && ap.x > ap.z)
    {
        glm::dvec3 uv = p / ap.x;
        if (p.x >= 0.0)
        {
            out.faceIndex = 0;
            out.uv.x = 0.5 * (uv.z + 1.0);
            out.uv.y = 0.5 * (uv.y + 1.0);
        }
        else
        {
            out.faceIndex = 1;
            out.uv.x = 0.5 * (uv.z + 1.0);
            out.uv.y = 0.5 * (uv.y + 1.0);
        }
    }
    if (ap.y > ap.x && ap.y > ap.z)
    {
        glm::dvec3 uv = p / ap.y;
        if (p.y >= 0.0)
        {
            out.faceIndex = 2;
            out.uv.x = 0.5 * (uv.x + 1.0);
            out.uv.y = 0.5 * (uv.z + 1.0);
        }
        else
        {
            out.faceIndex = 3;
            out.uv.x = 0.5 * (uv.x + 1.0);
            out.uv.y = 0.5 * (uv.z + 1.0);
        }
    }
    if (ap.z > ap.x && ap.z > ap.y)
    {
        glm::dvec3 uv = p / ap.z;
        if (p.z >= 0.0)
        {
            out.faceIndex = 4;
            out.uv.x = 0.5 * (uv.x + 1.0);
            out.uv.y = 0.5 * (uv.y + 1.0);
        }
        else
        {
            out.faceIndex = 5;
            out.uv.x = 0.5 * (uv.x + 1.0);
            out.uv.y = 0.5 * (uv.y + 1.0);
        }
    }

    return out;
}

TextureCoordinate sampleCube(const glm::dvec3 v)
{
    TextureCoordinate out;
    glm::dvec3 vAbs = abs(v);
    double ma;
    if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y)
    {
        out.faceIndex = v.z < 0.0 ? 5.0 : 4.0;
        ma = 0.5 / vAbs.z;
        out.uv = glm::dvec2(v.z < 0.0 ? -v.x : v.x, -v.y);
    }
    else if(vAbs.y >= vAbs.x)
    {
        out.faceIndex = v.y < 0.0 ? 3.0 : 2.0;
        ma = 0.5 / vAbs.y;
        out.uv = glm::dvec2(v.x, v.y < 0.0 ? -v.z : v.z);
    }
    else
    {
        out.faceIndex = v.x < 0.0 ? 1.0 : 0.0;
        ma = 0.5 / vAbs.x;
        out.uv = glm::dvec2(v.x < 0.0 ? v.z : -v.z, -v.y);
    }
    out.uv = out.uv * ma + 0.5;
    return out;
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
TextureCoordinate mapPoint(const glm::dvec3& p)
{
    //return sampleCube(p);
    double absX = fabs(p.x);
    double absY = fabs(p.y);
    double absZ = fabs(p.z);

    int isXPositive = p.x > 0 ? 1 : 0;
    int isYPositive = p.y > 0 ? 1 : 0;
    int isZPositive = p.z > 0 ? 1 : 0;

    double maxAxis = 0.0, uc = 0.0, vc = 0.0;

    int index = 0;
    // POSITIVE X
    if (isXPositive && absX >= absY && absX >= absZ)
    {

        // u (0 to 1) goes from +z to -z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = -p.z;
        vc =  p.y;
        index = 0;
    }

    // NEGATIVE X
    else if (!isXPositive && absX >= absY && absX >= absZ)
    {
        return doIt(p);

        // u (0 to 1) goes from -z to +z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = p.z;
        vc = p.y;
        index = 1;
    }
    // POSITIVE Y
    else if (isYPositive && absY >= absX && absY >= absZ)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from +z to -z
        maxAxis = absY;
        uc =  p.x;
        vc = -p.z;
        index = 2;
    }
    // NEGATIVE Y
    else if (!isYPositive && absY >= absX && absY >= absZ)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -z to +z
        maxAxis = absY;
        uc = p.x;
        vc = p.z;
        index = 3;
    }
    // POSITIVE Z
    else if (isZPositive && absZ >= absX && absZ >= absY)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = p.x;
        vc = p.y;
        index = 4;
    }
    // NEGATIVE Z
    else if (!isZPositive && absZ >= absX && absZ >= absY)
    {
        // u (0 to 1) goes from +x to -x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = -p.x;
        vc =  p.y;
        index = 5;
    }

    // Convert range from -1 to 1 to 0 to 1
    TextureCoordinate out;
    out.faceIndex = index;
    out.uv.x = 0.5 * (uc / maxAxis + 1.0);
    out.uv.y = 0.5 * (vc / maxAxis + 1.0);
    return out;
}

} // namespace texture_cube_mapping
} // namespace rasperi
} // namespace kuu
