#include "Imported.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

template<int n>
Imported<n>::Imported() : filepath("") {
}

template<int n>
Imported<n>::Imported(const std::string& filePath) : filepath(filePath) {
    loadGLTF(filePath);
}

template<int n>
Imported<n>::Imported(const std::string& filePath, const glm::vec<n, float>& centre, float scale)
    : filepath(filePath) {
    loadGLTF(filePath);

    // Move model to requested centre and scale it.
    glm::mat<n, n, float> scaleMat(1.0f);
    for (int i = 0; i < 3 && i < n; ++i) {
        scaleMat[i][i] = scale;
    }

    glm::mat<n, n, float> transMat = translation<n>(centre[0], centre[1], centre[2]);

    (*this) *= scaleMat;
    (*this) *= transMat;
}

template<int n>
Imported<n>::Imported(const Imported<n>& other)
    : Shape<n>(other), filepath(other.filepath), meshes(other.meshes) {
}

template<int n>
void Imported<n>::loadGLTF(const std::string& filePath) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        filePath,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType
    );

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    processNode(scene->mRootNode, scene);

    std::cout << "Loaded imported model: " << filePath
              << " with " << meshes.size() << " mesh(es)" << std::endl;
}

template<int n>
void Imported<n>::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

template<int n>
void Imported<n>::processMesh(aiMesh* aiMesh) {
    Mesh mesh;

    for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
        glm::vec<n, float> vertex(0.0f);

        vertex[0] = aiMesh->mVertices[i].x;
        vertex[1] = aiMesh->mVertices[i].y;
        vertex[2] = aiMesh->mVertices[i].z;

        if (n > 3) {
            vertex[3] = 1.0f;
        }

        mesh.vertices.push_back(vertex);

        if (aiMesh->HasNormals()) {
            mesh.normals.push_back(glm::vec3(
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            ));
        } else {
            mesh.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i) {
        aiFace face = aiMesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }

    meshes.push_back(mesh);
}

template<int n>
Imported<n>& Imported<n>::operator*=(const glm::mat<n, n, float>& m) {
    for (auto& mesh : meshes) {
        for (auto& vertex : mesh.vertices) {
            vertex = m * vertex;

            if (n > 3) {
                vertex[3] = 1.0f;
            }
        }
    }

    if (this->VAO != 0) {
        updateGLBuffers();
    }

    return *this;
}

template<int n>
Imported<n>* Imported<n>::operator*(const glm::mat<n, n, float>& m) const {
    Imported<n>* result = new Imported<n>(*this);
    (*result) *= m;
    return result;
}

template<int n>
float* Imported<n>::getPoints() const {
    int totalFloats = getNumPoints();

    if (totalFloats <= 0) {
        return nullptr;
    }

    float* points = new float[totalFloats];
    int index = 0;

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            for (int i = 0; i < n; ++i) {
                points[index++] = vertex[i];
            }
        }
    }

    return points;
}

template<int n>
int Imported<n>::getNumPoints() const {
    int totalFloats = 0;

    for (const auto& mesh : meshes) {
        totalFloats += static_cast<int>(mesh.vertices.size()) * n;
    }

    return totalFloats;
}

template<int n>
void Imported<n>::draw() {
    if (this->VAO == 0) {
        createGLBuffers();
    }

    glBindVertexArray(this->VAO);

    // Attribute 1 is used as constant colour in your project.
    glDisableVertexAttribArray(1);
    glVertexAttrib4f(
        1,
        this->colour[0],
        this->colour[1],
        this->colour[2],
        this->colour[3]
    );

    glDrawElements(GL_TRIANGLES, this->vertexCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

template<int n>
void Imported<n>::createGLBuffers(GLenum usage) {
    std::vector<glm::vec<n, float>> allVertices;
    std::vector<glm::vec3> allNormals;
    std::vector<unsigned int> allIndices;

    unsigned int vertexOffset = 0;

    for (const auto& mesh : meshes) {
        allVertices.insert(allVertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        allNormals.insert(allNormals.end(), mesh.normals.begin(), mesh.normals.end());

        for (unsigned int index : mesh.indices) {
            allIndices.push_back(index + vertexOffset);
        }

        vertexOffset += static_cast<unsigned int>(mesh.vertices.size());
    }

    if (allVertices.empty() || allIndices.empty()) {
        std::cout << "Imported model has no drawable geometry: " << filepath << std::endl;
        return;
    }

    this->vertexCount = static_cast<unsigned int>(allIndices.size());

    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    glGenBuffers(1, &this->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        allVertices.size() * sizeof(glm::vec<n, float>),
        allVertices.data(),
        usage
    );

    // Position attribute: location 0
    glVertexAttribPointer(
        0,
        n,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec<n, float>),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &this->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        allIndices.size() * sizeof(unsigned int),
        allIndices.data(),
        usage
    );

    // Normals should use location 2, not 1.
    // Location 1 is used for colour in your project.
    if (!allNormals.empty()) {
        glGenBuffers(1, &this->normVBO);
        glBindBuffer(GL_ARRAY_BUFFER, this->normVBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            allNormals.size() * sizeof(glm::vec3),
            allNormals.data(),
            usage
        );

        glVertexAttribPointer(
            2,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(glm::vec3),
            (void*)0
        );
        glEnableVertexAttribArray(2);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

template<int n>
void Imported<n>::updateGLBuffers(GLenum usage) {
    // Simple version: recreate buffers.
    // Only use this if your Shape base class has these fields.
    if (this->VAO != 0) {
        glDeleteVertexArrays(1, &this->VAO);
        this->VAO = 0;
    }

    if (this->VBO != 0) {
        glDeleteBuffers(1, &this->VBO);
        this->VBO = 0;
    }

    if (this->EBO != 0) {
        glDeleteBuffers(1, &this->EBO);
        this->EBO = 0;
    }

    if (this->normVBO != 0) {
        glDeleteBuffers(1, &this->normVBO);
        this->normVBO = 0;
    }

    createGLBuffers(usage);
}

template<int n>
GLenum Imported<n>::glDrawMode() const {
    return GL_TRIANGLES;
}

template<int n>
void Imported<n>::print() const {
    std::cout << "Imported Model: " << filepath << std::endl;
    std::cout << "Meshes: " << meshes.size() << std::endl;

    for (size_t i = 0; i < meshes.size(); ++i) {
        std::cout << "  Mesh " << i << ": "
                  << meshes[i].vertices.size() << " vertices, "
                  << meshes[i].indices.size() << " indices"
                  << std::endl;
    }
}

template<int n>
void Imported<n>::zoom(int percent) {
    float scale = 1.0f + (percent / 100.0f);

    glm::mat<n, n, float> scaleMat(1.0f);

    for (int i = 0; i < 3 && i < n; ++i) {
        scaleMat[i][i] = scale;
    }

    (*this) *= scaleMat;
}

template<int n>
void Imported<n>::rotate(int degrees) {
    float rad = degrees * 3.14159265358979323846f / 180.0f;

    glm::mat<n, n, float> rotMat(1.0f);

    float c = cosf(rad);
    float s = sinf(rad);

    rotMat[0][0] = c;
    rotMat[0][1] = -s;
    rotMat[1][0] = s;
    rotMat[1][1] = c;

    (*this) *= rotMat;
}

template<int n>
std::string Imported<n>::fprint() const {
    std::stringstream ss;
    ss << "Imported{" << filepath << ", meshes=" << meshes.size() << "}";
    return ss.str();
}