/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::TriangleClipper class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_triangle_clipper.h"
#include <array>
//#include <algorithm>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
//#include <glm/common.hpp>
//#include <glm/geometric.hpp>
//#include "bounding_box.h"
//#include "triangle.h"
//#include "rasperi_mesh.h"
#include "frustum.h"
#include "plane.h"
#include "ray.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
   A line-plane interesection with an error check.
 * ---------------------------------------------------------------- */
Vertex rayPlaneIntersection(const Plane& plane, const Ray& ray)
{
    Vertex out;
    auto res = plane.intersect(ray);
    if (res)
    {
        // this is just the position, what about normal, uv...?
        out.position    = res.pos;
        out.normal = plane.normal;
    }
    else
    {
        std::cerr << __FUNCTION__
                  <<": plane/ray intersection failed"
                  << std::endl;
    }

    return out;
}

/* ---------------------------------------------------------------- *
   Returns angle in radians between two edges of a point.
   See: https://stackoverflow.com/questions/43493711/
        the-angle-between-two-3d-vectors-with-a-result-range-0-360
 * ---------------------------------------------------------------- */
float angleBetweenThreeVertices(std::vector<glm::vec3> vertices,
                                glm::vec3 up)
{
    glm::vec3 v1 = vertices[1] - vertices[0];
    glm::vec3 v2 = vertices[2] - vertices[1];
    glm::vec3 cross = glm::cross(v1, v2);
    float dot = glm::dot(v1, v2);
    float angle = std::atan2(glm::length(cross), dot);

    float test = glm::dot(up, cross);
    if (test < 0.0f)
        angle = -angle;
    return angle;
}

//std::vector<kuu::Triangle> polygons(
//    std::vector<glm::vec3> poly)
//{
//    // At least four vertices required
//    if (poly.size() < 3)
//        return {};

//    // Polygon normal for using correct angle sign.
//    kuu::Triangle tri(poly[0], poly[1], poly[2]);
//    glm::vec3 n = tri.normal();
//    //vec3.fromValues(0, 0, 1)

//    std::vector<kuu::Triangle> triangles;
//    for (int i = 0; i < poly.size(); ++i)
//    {
//        // Find adjacent vertices
//        int i0 = i > 0 ? i - 1 : int(poly.size()) - 1;
//        int i1 = i;
//        int i2 = i < (poly.size() - 1) ? i + 1 : 0;
//        glm::vec3 v0 = poly[i0];
//        glm::vec3 v1 = poly[i1];
//        glm::vec3 v2 = poly[i2];

//        // Vertex must be convex
//        float angle = angleBetweenThreeVertices({ v0, v1, v2 }, n);
//        if (angle < 0.0 || angle >= M_PI)
//            continue;

//        // Vertex must be a eartip
//        kuu::Triangle tri(v0, v1, v2);
//        bool isEarTip = true;
//        for (int j = 0; j < poly.size(); ++j)
//        {
//            // Skip triangle indices
//            if (j == i0 || j == i1 || j == i2)
//                continue;

//            // Test vertex
//            glm::vec3 v = poly[j];
//            if (tri.contains(v))
//            {
//                isEarTip = false;
//                break;
//            }
//        }

//        if (isEarTip)
//        {
//            // Add a new triangle
//            triangles.push_back(kuu::Triangle(v1, v2, v0));
//            // Remove  the ear tip vertex
//            poly.erase(poly.begin() + i1);
//            //poly.splice(i1, 1);
//            // Start vertices  from the beginning
//            i = -1;
//            // Stop if all triangles have been found
//            if (poly.size() < 3)
//                break;
//        }
//    }

//    // Return triangles
//    return triangles;
//}

//std::vector<kuu::Triangle> clipPolygon(const std::vector<Vertex>& poly,
//                                       const std::vector<Plane>& planes)
//{
//    // Polygon must have atleast three vertices.
//    if (poly.size() < 2)
//        return {};

//    // Clip against the bounding box planes.
//    std::vector<Vertex> polygonOut = poly;
//    for (const Plane& plane : planes)
//    {
//        std::vector<Vertex> clipped = polygonOut;
//        polygonOut.clear();

//        Vertex start = clipped[clipped.size() - 1];
//        for (const Vertex& end : clipped)
//        {
//            // Get which side of the plane are the points
//            float startDistance = plane.distance(start.position);
//            float endDistance   = plane.distance(end.position);
//            bool startInside   = startDistance >= 0.0;
//            bool endInside     = endDistance   >= 0.0;

//            if (endInside)
//            {
//                if (!startInside)
//                {
//                    Ray r;
//                    r.start     = start.position;
//                    r.direction = glm::normalize(end.position - start.position);
//                    Vertex vert = rayPlaneIntersection(plane, r);
//                    polygonOut.push_back(vert);
//                }
//                polygonOut.push_back(end);
//            }
//            else if (startInside)
//            {
//                Ray r;
//                r.start     = start.position;
//                r.direction = glm::normalize(end.position - start.position);
//                Vertex vert = rayPlaneIntersection(plane, r);
//                polygonOut.push_back(vert);
//            }
//            start = end;
//        }

//        if (polygonOut.size() < 2)
//            return {};
//    }

//    // Reverse order to counter-clockwise
//    std::vector<Vertex> ccwPolygonOut;
//    ccwPolygonOut.resize(polygonOut.size());
//    std::reverse_copy(polygonOut.begin(),
//                      polygonOut.end(),
//                      ccwPolygonOut.begin());

//    std::vector<glm::vec3> points;
//    for (const Vertex& v : ccwPolygonOut)
//        points.push_back(v.position);

//    // Triangulate the polygon
//    return polygons(points);
//}

//std::vector<kuu::Triangle> TriangleClipper::ndcClip(std::vector<glm::vec3> tri)
//{
//    std::vector<Vertex> verts;
//    for (auto t : tri)
//    {
//        Vertex v;
//        v.position = t;
//        verts.push_back(v);
//    }

//    BoundingBox bb(glm::dvec3(-1), glm::dvec3(1));

//    std::vector<kuu::Triangle> ttt = clipPolygon(verts, bb.innerPlanes());
//    std::vector<kuu::Triangle> out;
//    for (auto t : ttt)
//    {
//        std::vector<Vertex> v;
//        v.push_back(Vertex(t.p0));
//        v.push_back(Vertex(t.p1));
//        v.push_back(Vertex(t.p2));

//        auto tris = polygons(v);
//        out.insert(out.end(), tris.begin(), tris.end());
//    }
//    return  out;
//}

//std::vector<kuu::Triangle> TriangleClipper::polygons(std::vector<Vertex> poly)
//{
//    // At least four vertices required
//    if (poly.size() < 3)
//        return {};

//    // Polygon normal for using correct angle sign.
//    kuu::Triangle tri(poly[0].position, poly[1].position, poly[2].position);
//    glm::vec3 n = tri.normal();
//    //vec3.fromValues(0, 0, 1)

//    std::vector<kuu::Triangle> triangles;
//    for (int i = 0; i < poly.size(); ++i)
//    {
//        // Find adjacent vertices
//        int i0 = i > 0 ? i - 1 : int(poly.size()) - 1;
//        int i1 = i;
//        int i2 = i < (poly.size() - 1) ? i + 1 : 0;
//        glm::vec3 v0 = poly[i0].position;
//        glm::vec3 v1 = poly[i1].position;
//        glm::vec3 v2 = poly[i2].position;

//        // Vertex must be convex
//        float angle = angleBetweenThreeVertices({ v0, v1, v2 }, n);
//        if (angle < 0.0 || angle >= M_PI)
//            continue;

//        // Vertex must be a eartip
//        kuu::Triangle tri(v0, v1, v2);
//        bool isEarTip = true;
//        for (int j = 0; j < poly.size(); ++j)
//        {
//            // Skip triangle indices
//            if (j == i0 || j == i1 || j == i2)
//                continue;

//            // Test vertex
//            glm::vec3 v = poly[j].position;
//            if (tri.contains(v))
//            {
//                isEarTip = false;
//                break;
//            }
//        }

//        if (isEarTip)
//        {
//            // Add a new triangle
//            triangles.push_back(kuu::Triangle(v1, v2, v0));
//            // Remove  the ear tip vertex
//            poly.erase(poly.begin() + i1);
//            //poly.splice(i1, 1);
//            // Start vertices  from the beginning
//            i = -1;
//            // Stop if all triangles have been found
//            if (poly.size() < 3)
//                break;
//        }
//    }

//    // Return triangles
//    return triangles;
//}


/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct TriangleClipper::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(const glm::dmat4& cameraMatrix,
         const glm::vec4& viewport)
    {
        // Extract frustum planes pointing inside
        Frustum frustum(cameraMatrix, viewport);
        /*
         * 0 near bottom left
         * 1 near bottom right
         * 2 near top left
         * 3 near top right
         * 4 far bottom left
         * 5 far bottom right
         * 6 far top left
         * 7 far top right
         * http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
         *
         */
        for (auto& plane : frustum.corners)
        {
            std::cout << "c " << glm::to_string(plane) << std::endl;
        }

        planes[0].point  = frustum.corners[0];
        planes[1].normal = glm::cross(frustum.corners[2] - frustum.corners[0],
                                      frustum.corners[4] - frustum.corners[0]);
        planes[1].point  = frustum.corners[5];
        planes[0].normal = glm::cross(frustum.corners[7] - frustum.corners[5],
                                      frustum.corners[1] - frustum.corners[5]);
        planes[2].point  = frustum.corners[2];
        planes[2].normal = glm::cross(frustum.corners[3] - frustum.corners[2],
                                      frustum.corners[6] - frustum.corners[2]);
        planes[3].point  = frustum.corners[0];
        planes[3].normal = glm::cross(frustum.corners[4] - frustum.corners[0],
                                      frustum.corners[1] - frustum.corners[0]);
        planes[4].point  = frustum.corners[1];
        planes[4].normal = glm::cross(frustum.corners[0] - frustum.corners[1],
                                      frustum.corners[3] - frustum.corners[1]);
        planes[5].point  = frustum.corners[4];
        planes[5].normal = glm::cross(frustum.corners[5] - frustum.corners[4],
                                      frustum.corners[6] - frustum.corners[4]);

        for (auto& plane : planes)
        {
            plane.normal = glm::normalize(plane.normal);
            std::cout << "plane " << glm::to_string(plane.normal) << std::endl;
        }
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::vector<Triangle> clip(const Triangle &tri) const
    {
        std::vector<Vertex> polygonOut;
        polygonOut.push_back(tri.p1);
        polygonOut.push_back(tri.p2);
        polygonOut.push_back(tri.p3);

        for (const Plane& plane : planes)
        {
            std::cout << "plane test: "
                      << glm::to_string(plane.point)  << ", "
                      << glm::to_string(plane.normal) << std::endl;

            std::vector<Vertex> clipped = polygonOut;
            polygonOut.clear();

            Vertex start = clipped[clipped.size() - 1];
            for (const Vertex& end : clipped)
            {
                // Get which side of the plane are the points
                float startDistance = plane.distance(start.position);
                float endDistance   = plane.distance(end.position);
                bool startInside   = startDistance >= 0.0f;
                bool endInside     = endDistance   >= 0.0f;
                std::cout << startDistance << ", " << endDistance << std::endl;

                if (endInside)
                {
                    if (!startInside)
                    {
                        Ray r;
                        r.start     = start.position;
                        r.direction = glm::normalize(end.position - start.position);
                        Vertex vert = rayPlaneIntersection(plane, r);
                        polygonOut.push_back(vert);
                    }
                    polygonOut.push_back(end);
                }
                else if (startInside)
                {
                    Ray r;
                    r.start     = start.position;
                    r.direction = glm::normalize(end.position - start.position);
                    Vertex vert = rayPlaneIntersection(plane, r);
                    polygonOut.push_back(vert);
                }
                start = end;
            }

            std::cout << polygonOut.size() << std::endl;
            if (polygonOut.size() < 2)
                return {};
        }

        // Reverse order to counter-clockwise
        std::vector<Vertex> ccwPolygonOut;
        ccwPolygonOut.resize(polygonOut.size());
        std::reverse_copy(polygonOut.begin(),
                          polygonOut.end(),
                          ccwPolygonOut.begin());

//        std::vector<glm::vec3> points;
//        for (const Vertex& v : ccwPolygonOut)
//            points.push_back(v.position);

        // Triangulate the polygon
        return polygons(ccwPolygonOut);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::vector<Triangle> polygons(std::vector<Vertex> poly) const
    {
        // At least four vertices required
        if (poly.size() < 3)
            return {};

        // Polygon normal for using correct angle sign.
        Triangle tri = { poly[0].position, poly[1].position, poly[2].position };
        const glm::dvec3 n = tri.normal();

        std::vector<Triangle> triangles;
        for (int i = 0; i < poly.size(); ++i)
        {
            // Find adjacent vertices
            size_t i0 = i > 0 ? i - 1 : int(poly.size()) - 1;
            size_t i1 = i;
            size_t i2 = i < (poly.size() - 1) ? i + 1 : 0;
            glm::dvec3 v0 = poly[i0].position;
            glm::dvec3 v1 = poly[i1].position;
            glm::dvec3 v2 = poly[i2].position;

            // Vertex must be convex
            float angle = angleBetweenThreeVertices({ v0, v1, v2 }, n);
            if (angle < 0.0 || angle >= M_PI)
                continue;

            // Vertex must be a eartip
            Triangle tri = { v0, v1, v2 };
            bool isEarTip = true;
            for (size_t j = 0; j < poly.size(); ++j)
            {
                // Skip triangle indices
                if (j == i0 || j == i1 || j == i2)
                    continue;

                // Test vertex
                glm::vec3 v = poly[j].position;
                if (tri.contains(v))
                {
                    isEarTip = false;
                    break;
                }
            }

            if (isEarTip)
            {
                // Add a new triangle
                triangles.push_back(Triangle(v1, v2, v0));
                // Remove  the ear tip vertex
                poly.erase(poly.begin() + i1);
                //poly.splice(i1, 1);
                // Start vertices  from the beginning
                i = -1;
                // Stop if all triangles have been found
                if (poly.size() < 3)
                    break;
            }
        }

        // Return triangles
        return triangles;
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    std::array<Plane, 6> planes;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
TriangleClipper::TriangleClipper(const glm::dmat4& cameraMatrix,
                                 const glm::vec4& viewport)
    : impl(std::make_shared<Impl>(cameraMatrix, viewport))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
std::vector<Triangle> TriangleClipper::clip(const Triangle &tri) const
{ return impl->clip(tri); }

} // namespace rasperi
} // namespace kuu
