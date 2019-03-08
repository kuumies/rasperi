/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::OpenGLTriangleMesh class.
 * ---------------------------------------------------------------- */
 
#include "rasperi_opengl_triangle_mesh.h"

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct OpenGLTriangleMesh::Impl
{
    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    Impl(OpenGLTriangleMesh* self, const Mesh& mesh)
        : self(self)
    {
        std::vector<float> vertices;
        for (const Vertex& v : mesh.vertices)
        {
            vertices.push_back(float(v.position.x));
            vertices.push_back(float(v.position.y));
            vertices.push_back(float(v.position.z));
            vertices.push_back(float(v.texCoord.x));
            vertices.push_back(float(v.texCoord.y));
            vertices.push_back(float(v.normal.x));
            vertices.push_back(float(v.normal.y));
            vertices.push_back(float(v.normal.z));
            vertices.push_back(float(v.tangent.x));
            vertices.push_back(float(v.tangent.y));
            vertices.push_back(float(v.tangent.z));
            vertices.push_back(float(v.bitangent.x));
            vertices.push_back(float(v.bitangent.y));
            vertices.push_back(float(v.bitangent.z));
        }
        const std::vector<unsigned>& indices = mesh.indices;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     GLsizei(vertices.size() * sizeof(float)),
                     vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     GLsizei(indices.size() * sizeof(unsigned)),
                     indices.data(),
                     GL_STATIC_DRAW);

        const int stride = 14 * sizeof(float);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                              BUFFER_OFFSET(0 * sizeof(float)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                              BUFFER_OFFSET(3 * sizeof(float)));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                              BUFFER_OFFSET(5 * sizeof(float)));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride,
                              BUFFER_OFFSET(8 * sizeof(float)));
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride,
                              BUFFER_OFFSET(11 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        indexCount = GLsizei(indices.size());
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    ~Impl()
    {
        glDeleteBuffers(1, &ibo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    void draw()
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    /* ------------------------------------------------------------ *
     * ------------------------------------------------------------ */
    OpenGLTriangleMesh* self;
    GLuint pgm;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLsizei indexCount;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
OpenGLTriangleMesh::OpenGLTriangleMesh(const Mesh& mesh)
    : impl(std::make_shared<Impl>(this, mesh))
{}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void OpenGLTriangleMesh::draw()
{ impl->draw(); }

} // namespace rasperi
} // namespace kuu
