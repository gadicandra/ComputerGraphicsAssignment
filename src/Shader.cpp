#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const char* path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Failed to open: " << path << "\n"; return ""; }
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vSrc = readFile(vertexPath);
    std::string fSrc = readFile(fragmentPath);
    const char* vCode = vSrc.c_str();
    const char* fCode = fSrc.c_str();

    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vCode, nullptr); glCompileShader(v);
    checkCompile(v, "VERTEX");

    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fCode, nullptr); glCompileShader(f);
    checkCompile(f, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, v); glAttachShader(ID, f);
    glLinkProgram(ID);
    checkCompile(ID, "PROGRAM");

    glDeleteShader(v); glDeleteShader(f);
}

void Shader::checkCompile(unsigned int s, const std::string& type) {
    int ok; char log[1024];
    if (type == "PROGRAM") {
        glGetProgramiv(s, GL_LINK_STATUS, &ok);
        if (!ok) { glGetProgramInfoLog(s, 1024, nullptr, log);
                   std::cerr << "PROGRAM LINK ERROR:\n" << log << "\n"; }
    } else {
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { glGetShaderInfoLog(s, 1024, nullptr, log);
                   std::cerr << type << " COMPILE ERROR:\n" << log << "\n"; }
    }
}

void Shader::setBool (const std::string& n, bool v)             const { glUniform1i (glGetUniformLocation(ID, n.c_str()), (int)v); }
void Shader::setInt  (const std::string& n, int v)              const { glUniform1i (glGetUniformLocation(ID, n.c_str()), v); }
void Shader::setFloat(const std::string& n, float v)            const { glUniform1f (glGetUniformLocation(ID, n.c_str()), v); }
void Shader::setVec3 (const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(ID, n.c_str()), 1, &v[0]); }
void Shader::setVec3 (const std::string& n, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID, n.c_str()), x, y, z); }
void Shader::setMat4 (const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }