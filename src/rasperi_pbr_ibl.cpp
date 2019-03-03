/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of types of kuu::rasperi::PbrIbl class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_pbr_ibl.h"
#include <array>
#include <functional>
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtGui/QPainter>
#include "rasperi_double_map.h"
#include "rasperi_framebuffer.h"
#include "rasperi_sampler.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class CubeCamera
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    CubeCamera(double aspectRatio)
    {
        // --------------------------------------------------------
        // Create a camera for each cubemap face

        std::array<glm::dquat, 6> rotations =
        {
            // pos x
            glm::angleAxis(glm::radians(-90.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

            // neg x
            glm::angleAxis(glm::radians( 90.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

            // pos y
            glm::angleAxis(glm::radians(-90.0), glm::dvec3(1.0, 0.0, 0.0)),

            // neg y
            glm::angleAxis(glm::radians( 90.0), glm::dvec3(1.0, 0.0, 0.0)),

            // pos z
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

            // neg z
            glm::angleAxis(glm::radians(  0.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),
        };

        const double fov       = M_PI * 0.5;
        const double nearPlane = 0.1;
        const double farPlane  = 150.0;
        projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

        for (size_t face = 0; face < 6; ++face)
            viewMatrices[face] = glm::mat4_cast(rotations[face]);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dmat4 cameraMatrix(size_t face) const
    { return projectionMatrix * viewMatrices[face]; }

    glm::dmat4 projectionMatrix;
    std::array<glm::dmat4, 6> viewMatrices;

};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class NdcCubeRasterizer
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    class BoundingBox
    {
    public:
        BoundingBox()
        {
            min.x =  std::numeric_limits<double>::max();
            min.y =  std::numeric_limits<double>::max();
            max.x = -std::numeric_limits<double>::max();
            max.y = -std::numeric_limits<double>::max();
        }

        void update(const glm::dvec2& p)
        {
            if (p.x < min.x) min.x = p.x;
            if (p.y < min.y) min.y = p.y;
            if (p.x > max.x) max.x = p.x;
            if (p.y > max.y) max.y = p.y;
        }

        glm::dvec2 min;
        glm::dvec2 max;
    };

    using Callback = std::function<void(glm::dvec3)>;

    int w;
    int h;
    Callback callback;
    CubeCamera cubeCamera;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    NdcCubeRasterizer(int w, int h, Callback callback)
        : w(w)
        , h(h)
        , callback(callback)
        , cubeCamera(w / double(h))
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run() const
    {
        // --------------------------------------------------------
        // Create NDC cube

        std::vector<glm::dvec3> vertexData =
        {
            { -1,-1,-1 },
            {  1, 1,-1 },
            {  1,-1,-1 },
            { -1, 1,-1 },
            { -1,-1, 1 },
            {  1,-1, 1 },
            {  1, 1, 1 },
            { -1, 1, 1 },
        };

        std::vector<unsigned> indexData
        {
            2,1,0, 3,0,1,
            6,5,4, 4,7,6,
            0,3,7, 7,4,0,
            1,2,6, 5,6,2,
            5,2,0, 0,4,5,
            1,6,3, 7,3,6,
        };

        // --------------------------------------------------------
        // Rasterize

        for (size_t face = 0; face < 6; ++face)
        {
            std::cout << "Process face " << face << std::endl;

            glm::dmat4 camera = cubeCamera.cameraMatrix(face);

            for (size_t i = 0; i < indexData.size(); i += 3)
            {
                glm::dvec3 v1 = vertexData[indexData[i+0]];
                glm::dvec3 v2 = vertexData[indexData[i+1]];
                glm::dvec3 v3 = vertexData[indexData[i+2]];

                glm::dvec3 proj1 = project(camera, v1);
                glm::dvec3 proj2 = project(camera, v2);
                glm::dvec3 proj3 = project(camera, v3);

                glm::dvec2 vp1 = viewportTransform(proj1);
                glm::dvec2 vp2 = viewportTransform(proj2);
                glm::dvec2 vp3 = viewportTransform(proj3);

                BoundingBox bb;
                bb.update(vp1);
                bb.update(vp2);
                bb.update(vp3);

                int xmin = std::max(0, std::min(w - 1, int(std::floor(bb.min.x))));
                int ymin = std::max(0, std::min(h - 1, int(std::floor(bb.min.y))));
                int xmax = std::max(0, std::min(w - 1, int(std::floor(bb.max.x))));
                int ymax = std::max(0, std::min(h - 1, int(std::floor(bb.max.y))));

//                #pragma omp parallel for
                for (int y = ymin; y <= ymax; ++y)
                for (int x = xmin; x <= xmax; ++x)
                {
                    glm::dvec2 screen(x + 0.5, y + 0.5);

                    // Barycentric weights
                    double w1 = edgeFunction(vp2, vp3, screen);
                    double w2 = edgeFunction(vp3, vp1, screen);
                    double w3 = edgeFunction(vp1, vp2, screen);
                    if (w1 < 0.0 || w2 < 0.0 || w3 < 0.0)
                        continue;

                    // Top-left edge rule
                    glm::dvec2 edge1 = vp2 - vp3;
                    glm::dvec2 edge2 = vp3 - vp1;
                    glm::dvec2 edge3 = vp1 - vp2;
                    bool overlaps = true;
                    overlaps &= (w1 == 0.0 ? ((edge1.y == 0.0 && edge1.x < 0.0) ||  edge1.y < 0.0) : (w1 > 0.0));
                    overlaps &= (w2 == 0.0 ? ((edge2.y == 0.0 && edge2.x < 0.0) ||  edge2.y < 0.0) : (w2 > 0.0));
                    overlaps &= (w3 == 0.0 ? ((edge3.y == 0.0 && edge3.x < 0.0) ||  edge3.y < 0.0) : (w3 > 0.0));
                    if (!overlaps)
                        continue;

                    double area = edgeFunction(vp1, vp2, vp3);
                    w1 /= area;
                    w2 /= area;
                    w3 /= area;

                    double z  = 1.0 / (w1 * 1.0 / proj1.z +
                                       w2 * 1.0 / proj2.z +
                                       w3 * 1.0 / proj3.z);

//                    // Depth test.
//                    double d = depthbuffer.get(x, y, 0);
//                    if (z >= d)
//                        continue;
//                    depthbuffer.set(x, y, 0, z);

                    glm::dvec3 p1 = v1 / proj1.z;
                    glm::dvec3 p2 = v2 / proj2.z;
                    glm::dvec3 p3 = v3 / proj3.z;
                    glm::dvec3 p = p1  * w1 * z +
                                   p2  * w2 * z +
                                   p3  * w3 * z;

                    callback(p);
                }
            }
        }
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    glm::dvec3 project(const glm::dmat4& m, const glm::dvec3& p) const
    {
        const glm::dvec4 v = m * glm::dvec4(p, 1.0);
        if (v.w == 0.0)
        {
            std::cerr << "proj err" << std::endl;
            return glm::dvec3(0.0);
        }
        return glm::dvec3(v.x / v.w, v.y / v.w, v.z / v.w);
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    glm::dvec2 viewportTransform(const glm::dvec3& p) const
    {
        const glm::ivec2 vp(w - 1, h - 1);
        const glm::dvec2 halfViewport = glm::dvec2(vp) * 0.5;

        glm::dvec2 out;
        out.x =        (p.x + 1.0) * halfViewport.x;
        out.y = vp.y - (p.y + 1.0) * halfViewport.y;

        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    double edgeFunction(const glm::dvec2& a,
                        const glm::dvec2& b,
                        const glm::dvec2& c) const
    {
        return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
    }
};

/* ---------------------------------------------------------------- *
   https://en.wikipedia.org/wiki/Cube_mapping
 * ---------------------------------------------------------------- */
class CubeMap
{
public:
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    CubeMap(int faceSize)
        : w(faceSize)
        , h(faceSize)
        //, data(4 * faceSize, 3 * faceSize, QImage::Format_RGB32)
    {
        for (size_t i = 0; i < faces.size(); ++i)
        {
            faces[i] = QImage(faceSize, faceSize, QImage::Format_RGB32);
            faces[i].fill(qRgba(255, 255, 255, 255));
        }
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void setFaceData(const QImage& map)
    {
        for (size_t i = 0; i < faces.size(); ++i)
        {
            QPainter p(&faces[i]);
            p.drawImage(faces[i].rect(), map.scaled(w, h, Qt::KeepAspectRatioByExpanding), faces[i].rect());
            //p.setBrush(QBrush(map));
            //p.drawRect(faces[i].rect());
        }
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void writeToFile(QString path)
    {
        QImage data(4 * w, 3 * h, QImage::Format_RGB32);
        data.fill(0);
        QPainter p(&data);
        p.drawImage(QRect(    w,     0, w, h), faces[0], QRect(0, 0, w, h)); // +Y
        p.drawImage(QRect(    0,     h, w, h), faces[1], QRect(0, 0, w, h)); // -X
        p.drawImage(QRect(    w,     h, w, h), faces[2], QRect(0, 0, w, h)); // +Z
        p.drawImage(QRect(2 * w,     h, w, h), faces[3], QRect(0, 0, w, h)); // +X
        p.drawImage(QRect(3 * w,     h, w, h), faces[4], QRect(0, 0, w, h)); // -Z
        p.drawImage(QRect(    w, 2 * h, w, h), faces[5], QRect(0, 0, w, h)); // -Z
        data.save(path);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    glm::dvec3 uvToXyz(int index, double u, double v) const
    {
        // convert range 0 to 1 to -1 to 1
        double uc = 2.0 * u - 1.0;
        double vc = 2.0 * v - 1.0;

        glm::dvec3 out;
        switch (index)
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

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::pair<int, glm::dvec2> xyzToUv(const glm::dvec3& p)
    {
        double absX = fabs(p.x);
        double absY = fabs(p.y);
        double absZ = fabs(p.z);

        int isXPositive = p.x > 0 ? 1 : 0;
        int isYPositive = p.y > 0 ? 1 : 0;
        int isZPositive = p.z > 0 ? 1 : 0;

        double maxAxis = 0.0, uc = 0.0, vc = 0.0;

        int index = 0;
        // POSITIVE X
        //if (isXPositive && absX >= absY && absX >= absZ)
        {
            // u (0 to 1) goes from +z to -z
            // v (0 to 1) goes from -y to +y
            maxAxis = absX;
            uc = -p.z;
            vc =  p.y;
            index = 0;
        }

        // NEGATIVE X
        if (!isXPositive && absX >= absY && absX >= absZ)
        {
            // u (0 to 1) goes from -z to +z
            // v (0 to 1) goes from -y to +y
            maxAxis = absX;
            uc = p.z;
            vc = p.y;
            index = 1;
        }
        // POSITIVE Y
        if (isYPositive && absY >= absX && absY >= absZ)
        {
            // u (0 to 1) goes from -x to +x
            // v (0 to 1) goes from +z to -z
            maxAxis = absY;
            uc =  p.x;
            vc = -p.z;
            index = 2;
        }
        // NEGATIVE Y
        if (!isYPositive && absY >= absX && absY >= absZ)
        {
            // u (0 to 1) goes from -x to +x
            // v (0 to 1) goes from -z to +z
            maxAxis = absY;
            uc = p.x;
            vc = p.z;
            index = 3;
        }
        // POSITIVE Z
        if (isZPositive && absZ >= absX && absZ >= absY)
        {
            // u (0 to 1) goes from -x to +x
            // v (0 to 1) goes from -y to +y
            maxAxis = absZ;
            uc = p.x;
            vc = p.y;
            index = 4;
        }
        // NEGATIVE Z
        if (!isZPositive && absZ >= absX && absZ >= absY)
        {
            // u (0 to 1) goes from +x to -x
            // v (0 to 1) goes from -y to +y
            maxAxis = absZ;
            uc = -p.x;
            vc =  p.y;
            index = 5;
        }

        // Convert range from -1 to 1 to 0 to 1
        std::pair<int, glm::dvec2> out;
        out.first = index;
        out.second.x = 0.5 * (uc / maxAxis + 1.0);
        out.second.y = 0.5 * (vc / maxAxis + 1.0);
        return out;
    }
    int w;
    int h;
    std::array<QImage, 6> faces;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct PbrIbl::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    class BoundingBox
    {
    public:
        BoundingBox()
        {
            min.x =  std::numeric_limits<double>::max();
            min.y =  std::numeric_limits<double>::max();
            max.x = -std::numeric_limits<double>::max();
            max.y = -std::numeric_limits<double>::max();
        }

        void update(const glm::dvec2& p)
        {
            if (p.x < min.x) min.x = p.x;
            if (p.y < min.y) min.y = p.y;
            if (p.x > max.x) max.x = p.x;
            if (p.y > max.y) max.y = p.y;
        }

        glm::dvec2 min;
        glm::dvec2 max;
    };


    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(PbrIbl* self, int size)
        : self(self)
        , size(size)
        , cubeMap(size)
        , irradianceMap(size)
        , depthbuffer(size, size, 1)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(const QImage& bgMap)
    {
        map = bgMap;
        cubeMap.setFaceData(map);

//        DoubleRgbCubeMap dcm2(size, size);
//        if (!dcm2.read("/temp/irradiance.dbl"))
//            std::cout << "Failed to read irradiance dbl" << std::endl;
//        dcm2.toQImage().save("/temp/00_im2.bmp");
//        return ;

        // --------------------------------------------------------
        // Create SDR cubemap of background map

//        cubeMap.setFaceData(map);

        // --------------------------------------------------------
        // Create a camera for each cubemap face

        std::array<glm::dquat, 6> rotations =
        {
            // pos x
            glm::angleAxis(glm::radians(-90.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

            // neg x
            glm::angleAxis(glm::radians( 90.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

            // pos y
            glm::angleAxis(glm::radians(-90.0), glm::dvec3(1.0, 0.0, 0.0)),

            // neg y
            glm::angleAxis(glm::radians( 90.0), glm::dvec3(1.0, 0.0, 0.0)),

            // pos z
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),

            // neg z
            glm::angleAxis(glm::radians(  0.0), glm::dvec3(0.0, 1.0, 0.0)) *
            glm::angleAxis(glm::radians(180.0), glm::dvec3(0.0, 0.0, 1.0)),
        };

        const double aspectRatio  = cubeMap.w / double(cubeMap.h);
        const double fov          = M_PI * 0.5;
        const double nearPlane    = 0.1;
        const double farPlane     = 150.0;

        std::array<std::pair<glm::dmat4, glm::dmat4>, 6> cameraMatrices;
        for (size_t face = 0; face < 6; ++face)
        {
            cameraMatrices[face].first = glm::mat4_cast(rotations[face]);
            cameraMatrices[face].second = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
        }

        // --------------------------------------------------------
        // Create NDC cube

        std::vector<glm::dvec3> vertexData =
        {
            { -1,-1,-1 },
            {  1, 1,-1 },
            {  1,-1,-1 },
            { -1, 1,-1 },
            { -1,-1, 1 },
            {  1,-1, 1 },
            {  1, 1, 1 },
            { -1, 1, 1 },
        };

        std::vector<unsigned> indexData
        {
            2,1,0, 3,0,1,
            6,5,4, 4,7,6,
            0,3,7, 7,4,0,
            1,2,6, 5,6,2,
            5,2,0, 0,4,5,
            1,6,3, 7,3,6,
        };

        // --------------------------------------------------------
        // Render irradiance map

        //DoubleRgbCubeMap dcm(size, size);

        auto irradienceCallback = [&](const glm::dvec3& p)
        {
            glm::dvec3 normal= glm::normalize(p);
            glm::dvec3 up    = glm::dvec3(0.0, 1.0, 0.0);
            glm::dvec3 right = glm::cross(up, normal);
                       up    = glm::cross(normal, right);

            glm::dvec3 irradiance = glm::dvec3(0.0);
            double sampleDelta = 0.025;
            //double sampleDelta = 0.4;
            double nrSamples = 0.0;
            for(double  phi = 0.0; phi < 2.0 * M_PI; phi += sampleDelta)
            {
                for(double  theta = 0.0; theta < 0.5 * M_PI; theta += sampleDelta)
                {
                    // spherical to cartesian (in tangent space)
                    glm::dvec3 tangentSample = glm::dvec3(sin(theta) * cos(phi),
                                                          sin(theta) * sin(phi),
                                                          cos(theta));
                    // tangent space to world
                    glm::dvec3 sampleVec = tangentSample.x * right +
                                           tangentSample.y * up +
                                           tangentSample.z * normal;

                    std::pair<int, glm::dvec2> texCoord = cubeMap.xyzToUv(sampleVec); // !!
                    Sampler sampler(cubeMap.faces[texCoord.first]);
                    sampler.setFilter(Sampler::Filter::Nearest);
                    glm::dvec3 texColor = sampler.sampleRgba(texCoord.second) * 20.0;
//                            irradiance = texColor;
                    irradiance += texColor * cos(theta) * sin(theta);
                    nrSamples++;
                }
            }
            irradiance = M_PI * irradiance * (1.0 / double(nrSamples));

            std::pair<int, glm::dvec2> texCoord2 = cubeMap.xyzToUv(normal);

            glm::ivec2 sc = mapCoord(texCoord2.second);
            self->irradiance.set(size_t(texCoord2.first), sc.x, sc.y, irradiance);
        };

        NdcCubeRasterizer rasterizer(size, size, irradienceCallback);
        rasterizer.run();

        // --------------------------------------------------------
        // Render prefilter map

        auto prefilterCallback = [&](const glm::dvec3& p)
        {

        };

        rasterizer.callback = prefilterCallback;
        rasterizer.run();

        //dcm.write("/temp/irradiance.dbl");
        //dcm.toQImage().save("/temp/00_imX.bmp");

#if 0
        for (size_t face = 0; face < 6; ++face)
        {
            std::cout << "Process face " << face << std::endl;

            depthbuffer.set(std::numeric_limits<double>::max());

            std::pair<glm::dmat4, glm::dmat4> cam = cameraMatrices[face];

            glm::dmat4 view       = cam.first;
            glm::dmat4 projection = cam.second;
            glm::dmat4 camera = projection * view;

            for (size_t i = 0; i < indexData.size(); i += 3)
            {
                glm::dvec3 v1 = vertexData[indexData[i+0]];
                glm::dvec3 v2 = vertexData[indexData[i+1]];
                glm::dvec3 v3 = vertexData[indexData[i+2]];

                glm::dvec3 proj1 = project(camera, v1);
                glm::dvec3 proj2 = project(camera, v2);
                glm::dvec3 proj3 = project(camera, v3);

                glm::dvec2 vp1 = viewportTransform(proj1);
                glm::dvec2 vp2 = viewportTransform(proj2);
                glm::dvec2 vp3 = viewportTransform(proj3);

                BoundingBox bb;
                bb.update(vp1);
                bb.update(vp2);
                bb.update(vp3);

                int xmin = std::max(0, std::min(size - 1, int(std::floor(bb.min.x))));
                int ymin = std::max(0, std::min(size - 1, int(std::floor(bb.min.y))));
                int xmax = std::max(0, std::min(size - 1, int(std::floor(bb.max.x))));
                int ymax = std::max(0, std::min(size - 1, int(std::floor(bb.max.y))));

                // the triangle is out of screen
//                if (xmin > 64 - 1 || xmax < 0 || ymin > size - 1 || ymax < 0)
//                    continue;

//                #pragma omp parallel for
                for (int y = ymin; y <= ymax; ++y)
                for (int x = xmin; x <= xmax; ++x)
//                for (int y = bb.min.y; y <= bb.max.y; ++y)
//                for (int x = bb.min.x; x <= bb.max.x; ++x)
//                for (int y = 0; y < 64; ++y)
//                for (int x = 0; x < 64; ++x)
                {
                    glm::dvec2 screen(x + 0.5, y + 0.5);

                    // Barycentric weights
                    double w1 = edgeFunction(vp2, vp3, screen);
                    double w2 = edgeFunction(vp3, vp1, screen);
                    double w3 = edgeFunction(vp1, vp2, screen);
                    if (w1 < 0.0 || w2 < 0.0 || w3 < 0.0)
                        continue;

                    // Top-left edge rule
                    glm::dvec2 edge1 = vp2 - vp3;
                    glm::dvec2 edge2 = vp3 - vp1;
                    glm::dvec2 edge3 = vp1 - vp2;
                    bool overlaps = true;
                    overlaps &= (w1 == 0.0 ? ((edge1.y == 0.0 && edge1.x < 0.0) ||  edge1.y < 0.0) : (w1 > 0.0));
                    overlaps &= (w2 == 0.0 ? ((edge2.y == 0.0 && edge2.x < 0.0) ||  edge2.y < 0.0) : (w2 > 0.0));
                    overlaps &= (w3 == 0.0 ? ((edge3.y == 0.0 && edge3.x < 0.0) ||  edge3.y < 0.0) : (w3 > 0.0));
                    if (!overlaps)
                        continue;

                    double area = edgeFunction(vp1, vp2, vp3);
                    w1 /= area;
                    w2 /= area;
                    w3 /= area;

                    double z  = 1.0 / (w1 * 1.0 / proj1.z +
                                       w2 * 1.0 / proj2.z +
                                       w3 * 1.0 / proj3.z);

                    // Depth test.
                    double d = depthbuffer.get(x, y, 0);
                    if (z >= d)
                        continue;
                    depthbuffer.set(x, y, 0, z);

                    glm::dvec3 p1 = v1 / proj1.z;
                    glm::dvec3 p2 = v2 / proj2.z;
                    glm::dvec3 p3 = v3 / proj3.z;
                    glm::dvec3 p = p1  * w1 * z +
                                   p2  * w2 * z +
                                   p3  * w3 * z;

//                    std::cout << __FUNCTION__
//                              << ": "
//                              << glm::to_string(p)
//                              << std::endl;

                    glm::dvec3 normal = normalize(p);

//                    std::array<glm::dvec4, 6> faceColors =
//                    {
//                        glm::dvec4(1.0, 0.0, 0.0, 1.0),
//                        glm::dvec4(0.0, 1.0, 0.0, 1.0),
//                        glm::dvec4(0.0, 0.0, 1.0, 1.0),
//                        glm::dvec4(1.0, 1.0, 0.0, 1.0),
//                        glm::dvec4(0.0, 1.0, 1.0, 1.0),
//                        glm::dvec4(1.0, 0.0, 1.0, 1.0),
//                    };

//                    std::pair<int, glm::dvec2> texCoord = irradianceMap.xyzToUv(normal);
                    //if (texCoord.first != face)
                    //    continue;

//                    glm::ivec2 tc = mapCoord(texCoord.second);
//                    if (tc != glm::ivec2(x, y))
//                    {
//                        std::cout << "Err "
//                                     << x << ", "
//                                     << y << ", "
//                                     << tc.x << ", "
//                                     << tc.y
//                                     << std::endl;
//                    }

//                    glm::dvec4 outColor = faceColors[texCoord.first];
//                    std::cout << __FUNCTION__
//                              << ": "
//                              << face << ", "
//                              << texCoord.first << ", "
//                              << x << ", "
//                              << y << ", "
//                              << glm::to_string(texCoord.second) << ", "
//                              << glm::to_string(glm::ivec2(int(std::floor(texCoord.second.x * (64 - 1))),
//                                                           int(std::floor(texCoord.second.y * (64 - 1))))) << ", "
//                              << glm::to_string(normal)
//                              << std::endl;

//                    writeRgba(irradianceMap.faces[texCoord.first], texCoord.second, outColor);

#if 1
                    glm::dvec3 irradiance = glm::dvec3(0.0);

                    glm::dvec3 up    = glm::dvec3(0.0, 1.0, 0.0);
                    glm::dvec3 right = glm::cross(up, normal);
                               up    = glm::cross(normal, right);

                    double sampleDelta = 0.025;
                    //double sampleDelta = 0.1;
                    double nrSamples = 0.0;
                    for(double  phi = 0.0; phi < 2.0 * M_PI; phi += sampleDelta)
                    {
                        for(double  theta = 0.0; theta < 0.5 * M_PI; theta += sampleDelta)
                        {
                            // spherical to cartesian (in tangent space)
                            glm::dvec3 tangentSample = glm::dvec3(sin(theta) * cos(phi),
                                                                  sin(theta) * sin(phi),
                                                                  cos(theta));
                            // tangent space to world
                            glm::dvec3 sampleVec = tangentSample.x * right +
                                                   tangentSample.y * up +
                                                   tangentSample.z * normal;

                            std::pair<int, glm::dvec2> texCoord = cubeMap.xyzToUv(normal);
                            Sampler sampler(cubeMap.faces[texCoord.first]);
                            sampler.setFilter(Sampler::Filter::Nearest);
                            glm::dvec3 texColor = sampler.sampleRgba(texCoord.second) * 20.0;
//                            irradiance = texColor;
                            irradiance += texColor * cos(theta) * sin(theta);
                            nrSamples++;
                        }
                    }
                    irradiance = M_PI * irradiance * (1.0 / double(nrSamples));
//                    irradiance = irradiance / (irradiance + glm::dvec3(1.0));

                    std::pair<int, glm::dvec2> texCoord2 = cubeMap.xyzToUv(normal);
                    //glm::dvec4 outColor = glm::dvec4(irradiance, 1.0);
                    //writeRgba(irradianceMap.faces[texCoord2.first], texCoord2.second, outColor);

                    glm::ivec2 sc = mapCoord(texCoord2.second);
                    dcm.set(size_t(texCoord2.first), sc.x, sc.y, irradiance);

                    //Sampler outSampler(cubeMap.faces[texCoord.first]);
                    //outSampler.writeRgba(texCoord.second, outColor);

                    //auto uv = irradianceMap.xyzToUv(normal)
                    //irradianceMap.data.setPixel()
#endif
                }
            }
        }

        dcm.write("/temp/irradiance.dbl");
        dcm.toQImage().save("/temp/00_im.bmp");

        cubeMap.writeToFile("/temp/00_cm.bmp");
        //irradianceMap.writeToFile("/temp/00_im.bmp");
#endif
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    glm::dvec3 project(const glm::dmat4& m, const glm::dvec3& p)
    {
        const glm::dvec4 v = m * glm::dvec4(p, 1.0);
        if (v.w == 0.0)
        {
            std::cerr << "proj err" << std::endl;
            return glm::dvec3(0.0);
        }
        return glm::dvec3(v.x / v.w, v.y / v.w, v.z / v.w);
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    glm::dvec2 viewportTransform(const glm::dvec3& p)
    {
        const glm::ivec2 vp(size - 1, size - 1);
        const glm::dvec2 halfViewport = glm::dvec2(vp) * 0.5;

        glm::dvec2 out;
        out.x =        (p.x + 1.0) * halfViewport.x;
        out.y = vp.y - (p.y + 1.0) * halfViewport.y;

        return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    double edgeFunction(const glm::dvec2& a,
                        const glm::dvec2& b,
                        const glm::dvec2& c)
    {
        return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void writeRgba(QImage& map,
                   const glm::dvec2& texCoord,
                   const glm::dvec4& rgba)
    {
        //glm::dvec2 uv = clampTexCoord(texCoord);
        glm::ivec2 sc = mapCoord(texCoord);
        //std::cout << __FUNCTION__ << ": " << sc.x << ", " << sc.y << std::endl;

        if (map.isNull() ||
            map.width() <= 0 ||
            map.height() <= 0 ||
            sc.x < 0 || sc.x >= map.width() ||
            sc.y < 0 || sc.y >= map.height())
        {
            std::cerr << __FUNCTION__ << ": " << "map error" << std::endl;
            std::cerr << __FUNCTION__ << ": "
                      << sc.x << ", " << sc.y << ", "
                      << map.width() << ", " << map.height()
                      << std::endl << std::flush;
            return;
        }

//        std::cout << __FUNCTION__ << ": " << sc.x << ", " << sc.y << std::endl;

        QRgb* line = reinterpret_cast<QRgb*>(map.scanLine(sc.y));
        line[sc.x] = qRgba(qRound(rgba.r * 255.0),
                           qRound(rgba.g * 255.0),
                           qRound(rgba.b * 255.0),
                           qRound(rgba.a * 255.0));
//        line[sc.x] = qRgba(0, 0, 255, 255);
        map.setPixel(sc.x, sc.y, qRgba(qRound(rgba.r * 255.0),
                                       qRound(rgba.g * 255.0),
                                       qRound(rgba.b * 255.0),
                                       qRound(rgba.a * 255.0)));
    }

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::dvec2 clampTexCoord(glm::dvec2 texCoord) const
    {
        texCoord.x = glm::clamp(texCoord.x, 0.0, 1.0);
        texCoord.y = glm::clamp(texCoord.y, 0.0, 1.0);
        return texCoord;
    }

    /* ----------------------------------------------------------- *
     * ----------------------------------------------------------- */
    glm::ivec2 mapCoord(glm::dvec2 texCoord) const
    {
        texCoord.y = 1.0 - texCoord.y;
        int px = int(std::floor(texCoord.x * double(size - 1)));
        int py = int(std::floor(texCoord.y * double(size - 1)));
        return glm::ivec2(px, py);
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    bool read(const QDir& dir)
    {
        bool ok = true;
        ok &= self->irradiance.read(dir.absoluteFilePath("irradiance.dbl"));
        ok &= self->prefilter.read(dir.absoluteFilePath("prefilter.dbl"));
        ok &= self->brdfIntegration.read(dir.absoluteFilePath("brdfIntegration.dbl"));
        return ok;
    }

    /* ---------------------------------------------------------------- *
     * ---------------------------------------------------------------- */
    bool write(const QDir& dir)
    {
        bool ok = true;
        ok &= self->irradiance.write(dir.absoluteFilePath("irradiance.dbl"));
        ok &= self->prefilter.write(dir.absoluteFilePath("prefilter.dbl"));
        ok &= self->brdfIntegration.write(dir.absoluteFilePath("brdfIntegration.dbl"));
        return ok;
    }

    PbrIbl* self;
    QImage map;
    int size;
    CubeMap cubeMap;
    CubeMap irradianceMap;
    DepthFramebuffer depthbuffer;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
PbrIbl::PbrIbl(int size)
    : irradiance(size, size)
    , prefilter(size, size)
    , brdfIntegration(size, size)
    , impl(std::make_shared<Impl>(this, size))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIbl::read(const QDir& dir)
{ return impl->read(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
bool PbrIbl::write(const QDir& dir)
{ return impl->write(dir); }

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void PbrIbl::run(const QImage& bgMap)
{ impl->run(bgMap); }

} // namespace rasperi
} // namespace kuu
