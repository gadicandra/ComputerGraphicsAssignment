#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include "Mesh.h"
#include "Shader.h"

class Model {
public:
    Model(const std::string& path) { loadModel(path); }
    void Draw(const Shader& shader) const {
        for (const auto& m : meshes) m.Draw(shader);
    }

private:
    std::vector<Mesh> meshes;
    std::string       directory;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};