#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use() const { glUseProgram(ID); }

    void setBool (const std::string& n, bool v)              const;
    void setInt  (const std::string& n, int v)               const;
    void setFloat(const std::string& n, float v)             const;
    void setVec3 (const std::string& n, const glm::vec3& v)  const;
    void setVec3 (const std::string& n, float x, float y, float z) const;
    void setMat4 (const std::string& n, const glm::mat4& m)  const;

private:
    static void checkCompile(unsigned int shader, const std::string& type);
};