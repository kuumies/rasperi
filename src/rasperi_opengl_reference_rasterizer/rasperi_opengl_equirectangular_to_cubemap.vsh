/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   OpenGL equirectangular to cubemap vertex shader.
 * ---------------------------------------------------------------- */
 
#version 330 core

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
layout(location = 0) in vec3 position;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
struct Matrices
{
    mat4 view;
    mat4 projection;
};

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
uniform Matrices matrices;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
out vec3 texCoord;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
void main()
{
    texCoord = position;
    gl_Position = matrices.projection *
                  matrices.view *
                  vec4(position, 1.0);
}
