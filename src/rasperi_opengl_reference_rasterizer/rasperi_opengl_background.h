/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::OpenGLBackground class.
 * ---------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vector>
#include <glad/glad.h>

class QImage;

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class OpenGLBackground
{
public:
    OpenGLBackground(const QImage& bg);

    GLuint tex() const;
    void draw();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
