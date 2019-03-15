/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   OpenGL PBR IBL irradiance vertex shader.
 * ---------------------------------------------------------------- */
 
#version 330 core

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

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