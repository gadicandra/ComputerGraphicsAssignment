#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO = 0;

    Mesh(std::vector<Vertex> v, std::vector<unsigned int> i);
    void Draw(const Shader& shader) const;

private:
    unsigned int VBO = 0, EBO = 0;
    void setupMesh();
};