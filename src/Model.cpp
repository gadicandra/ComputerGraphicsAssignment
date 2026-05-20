#include "Model.h"
#include <iostream>

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate     |  // konversi semua face ke triangle
        aiProcess_GenSmoothNormals|  // bikin normal kalau tidak ada
        aiProcess_FlipUVs         |  // OpenGL UV origin di kiri-bawah (kebalikan kebanyakan tool)
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
    std::cout << "Loaded model: " << path << " (" << meshes.size() << " meshes)\n";
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene*) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v{};
        v.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        v.Normal   = mesh->HasNormals()
                     ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                     : glm::vec3(0.0f, 1.0f, 0.0f);
        v.TexCoords = mesh->mTextureCoords[0]
                      ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                      : glm::vec2(0.0f);
        vertices.push_back(v);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    return Mesh(std::move(vertices), std::move(indices));
}