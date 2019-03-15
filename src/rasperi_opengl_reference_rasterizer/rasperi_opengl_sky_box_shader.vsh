/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   OpenGL PBR IBL irradiance vertex shader.
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

    mat4 rotView = mat4(mat3(matrices.view));
    vec4 clipPos = matrices.projection *
                   rotView *
                   vec4(position, 1.0);
    gl_Position = clipPos.xyww;
}
