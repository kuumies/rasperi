/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::rasperi::opengl::shader_loader namespace.
 * ---------------------------------------------------------------- */

#include "rasperi_opengl_shader_loader.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace kuu
{
namespace rasperi
{
namespace opengl_shader_loader
{
namespace 
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
std::string readTextFile(const std::string& path)
{
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
GLuint loadShader(const GLenum type, const std::string& shaderPath)
{
    std::string source = readTextFile(shaderPath);
    if (source.size() == 0)
    {
        std::cerr << "Shader source "
                  << shaderPath
                  << " is empty string"
                  << std::endl;
        return 0;
    }
    const char* cts = source.c_str();

    GLuint shr = glCreateShader(type);
    glShaderSource(shr, 1, &cts, nullptr);
    glCompileShader(shr);
    
    GLint status = GL_FALSE;
    glGetShaderiv(shr, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint length = 0;
        glGetShaderiv(shr, GL_INFO_LOG_LENGTH, &length);

        std::string log;
        log.resize(size_t(length + 1));

        char* logData = const_cast<char*>(log.c_str());
        GLchar* glLogData = reinterpret_cast<GLchar*>(logData);
        glGetShaderInfoLog(shr, length, nullptr, glLogData);
    
        std::string msg;
        msg += "Failed to compile shader.\n";
        msg += std::string("Filepath: ") += std::string(shaderPath) += "\n";
        msg += std::string("Log: ")      += log  += "\n";
        msg += std::string("Source: ")   += source;
        std::cerr << msg << std::endl;
        return 0;
    }
    
    return shr;
}
    
} // anonymous namespace

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
GLuint load(const std::string& vshPath,
            const std::string& fshPath)
{
    GLuint vsh = loadShader(GL_VERTEX_SHADER,   vshPath);
    GLuint fsh = loadShader(GL_FRAGMENT_SHADER, fshPath);
    
    GLuint pgm = glCreateProgram();
    glAttachShader(pgm, vsh);
    glAttachShader(pgm, fsh);
    glLinkProgram(pgm);
    
    GLint linkStatus = 0;
    glGetProgramiv(pgm, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        GLint length;
        glGetProgramiv(pgm, GL_INFO_LOG_LENGTH, &length);
        
        std::string log;
        log.resize(size_t(length + 1));

        char* logData = const_cast<char*>(log.c_str());
        GLchar* glLogData = reinterpret_cast<GLchar*>(logData);
        glGetProgramInfoLog(pgm, length, nullptr, glLogData);
                        
        std::string msg;
        msg += "Failed to link program.\n";
        msg += std::string("Log: ") += log  += "\n";
        std::cerr << msg << std::endl;
    }

    glDeleteShader(vsh);
    glDeleteShader(fsh);
    
    return pgm;
}

} // namespace opengl_shader_loader
} // namespace rasperi
} // namespace kuu
