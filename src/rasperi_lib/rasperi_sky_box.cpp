/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::SkyBox class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_sky_box.h"
#include <functional>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec4.hpp>
#include "rasperi_framebuffer.h"
#include "rasperi_texture_cube.h"
#include "rasperi_texture_cube_mapping.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class SkyBoxRasterizer
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

    using Callback = std::function<glm::dvec4(glm::dvec3)>;

    int w;
    int h;
    Callback callback;

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    SkyBoxRasterizer(Callback callback)
        : callback(callback)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(const glm::dmat4& camera,
             const glm::ivec2& viewportSize,
             Framebuffer& framebuffer)
    {
        w = viewportSize.x;
        h = viewportSize.y;

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

        bool windingCCW = true;
        std::vector<unsigned> indexData;
        if (windingCCW)
        {
            indexData =
            {
                0,1,2, 1,0,3,
                4,5,6, 6,7,4,
                7,3,0, 0,4,7,
                6,2,1, 2,6,5,
                0,2,5, 5,4,0,
                3,6,1, 6,3,7,
            };
        }
        else
        {
            indexData =
            {
                2,1,0, 3,0,1,
                6,5,4, 4,7,6,
                0,3,7, 7,4,0,
                1,2,6, 5,6,2,
                5,2,0, 0,4,5,
                1,6,3, 7,3,6,
            };
        }

//        std::vector<unsigned> indexData
//        {
//            2,1,0, 3,0,1,
//            6,5,4, 4,7,6,
//            0,3,7, 7,4,0,
//            1,2,6, 5,6,2,
//            5,2,0, 0,4,5,
//            1,6,3, 7,3,6,
//        };

        // --------------------------------------------------------
        // Rasterize

        #pragma omp parallel for
        for (int i = 0; i < indexData.size(); i += 3)
        {
            size_t ii = size_t(i); // OpenMP does not accept unsigned index in for loop
            glm::dvec3 v1 = vertexData[indexData[ii+0]];
            glm::dvec3 v2 = vertexData[indexData[ii+1]];
            glm::dvec3 v3 = vertexData[indexData[ii+2]];

            const glm::dvec3 triNormal =
                glm::normalize(glm::cross(v2 - v1,
                                          v3 - v1));


            std::vector<glm::dvec3> splitTriangle = split(v1, v2, v3, camera);
            if (splitTriangle.empty())
                continue;

            for (size_t j = 0; j < splitTriangle.size(); j += 3)
            {
                v1 = splitTriangle[j + 0];
                v2 = splitTriangle[j + 1];
                v3 = splitTriangle[j + 2];

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

    //                // Depth test.
//                    double d = framebuffer.depthTex.pixel(x, y)[0];
                    double z  = 1.0 / (w1 * 1.0 / proj1.z +
                                       w2 * 1.0 / proj2.z +
                                       w3 * 1.0 / proj3.z);
//                    if (z >= d)
//                        continue;

//                    std::array<double, 1> depthPix = { z };
//                    framebuffer.depthTex.setPixel(x, y, depthPix);

                    glm::dvec3 p1 = v1 / proj1.z;
                    glm::dvec3 p2 = v2 / proj2.z;
                    glm::dvec3 p3 = v3 / proj3.z;
                    glm::dvec3 p = p1  * w1 * z +
                                   p2  * w2 * z +
                                   p3  * w3 * z;

                    glm::dvec4 color = callback(p);
                    std::array<uchar, 4> colorPix =
                    { uchar(color.r * 255.0),
                      uchar(color.g * 255.0),
                      uchar(color.b * 255.0),
                      uchar(color.a * 255.0) };

    //                std::array<uchar, 4> colorPix =
    //                { 255,
    //                  255,
    //                  255,
    //                  255 };

                    framebuffer.colorTex.setPixel(x, y, colorPix);
                }
            }
        }
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::vector<glm::dvec3> split(glm::dvec3 p1, glm::dvec3 p2, glm::dvec3 p3, const glm::dmat4& m) const
    {
        std::cout << glm::to_string(p1) << ", "
                  << glm::to_string(p2) << ", "
                  << glm::to_string(p3) << std::endl;

        return { p1, p2, p3 };
//
//        glm::dvec4 pp1 = m * glm::dvec4(p1, 1.0);
//        glm::dvec4 pp2 = m * glm::dvec4(p2, 1.0);
//        glm::dvec4 pp3 = m * glm::dvec4(p3, 1.0);
//
//        kuu::BoundingBox bb(glm::dvec3(-1, -1, -1),
//                            glm::dvec3( 1,  1,  1));
//
//        Vertex v1;
//        v1.position = pp3;
//        Vertex v2;
//        v2.position = pp2;
//        Vertex v3;
//        v3.position = pp1;
//
//        std::vector<Vertex> poly = { v1, v2, v3  };
//        auto res = clipPolygon(poly, bb.innerPlanes());
//        qDebug() << res.size();
//        //if (res.empty())
//
//        //glm::dmat4 im = glm::inverse(m);
//        //std::vector<glm::dvec3> out;
        //for (auto tri : res)
        //{
        //    out.push_back(glm::dvec4(tri.p0, 1.0) * im);
        //    out.push_back(glm::dvec4(tri.p1, 1.0) * im);
        //    out.push_back(glm::dvec4(tri.p2, 1.0) * im);
        //}
        //return out;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
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

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
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
 * ---------------------------------------------------------------- */
struct SkyBox::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(SkyBox* self)
        : self(self)
    {}

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void run(const TextureCube<double, 4>& sky,
             const glm::dmat4& camera,
             const glm::ivec2& viewportSize,
             Framebuffer& framebuffer)
    {
         // --------------------------------------------------------
        // Render

        auto shadeCallback = [&](const glm::dvec3& p)
        {
            glm::dvec3 normal = glm::normalize(p);

            const texture_cube_mapping::TextureCoordinate texCoord =
                texture_cube_mapping::mapPoint(normal);

            const std::array<double, 4> pix = sky.face(size_t(texCoord.faceIndex)).pixel(texCoord.uv.x, texCoord.uv.y);
            glm::dvec3 color(pix[0], pix[1], pix[2]);
            color = color / (color + glm::dvec3(1.0));
            color = pow(color, glm::dvec3(1.0 / 2.2));
            return glm::dvec4(color, 1.0);
        };

        SkyBoxRasterizer rasterizer(shadeCallback);
        rasterizer.run(camera, viewportSize, framebuffer);
    }

    SkyBox* self;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
SkyBox::SkyBox()
    : impl(std::make_shared<Impl>(this))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void SkyBox::run(const TextureCube<double, 4>& sky,
                 const glm::dmat4& camera,
                 const glm::ivec2& viewportSize,
                 Framebuffer& framebuffer)
{ impl->run(sky, camera, viewportSize, framebuffer); }

} // namespace rasperi
} // namespace kuu
