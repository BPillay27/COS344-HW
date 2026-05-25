#ifndef IMPORTED_H
#define IMPORTED_H

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <assimp/scene.h>

#include "Shape.h"
#include "transformation.h"

template<int n>
class Imported : public Shape<n> {
private:
    struct Mesh {
        std::vector<glm::vec<n, float>> vertices;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
    };

    std::string filepath;
    std::vector<Mesh> meshes;

    void loadGLTF(const std::string& filePath);
    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* aiMesh);

public:
    Imported();
    Imported(const std::string& filePath);
    Imported(const std::string& filePath, const glm::vec<n, float>& centre, float scale);
    Imported(const Imported<n>& other);

    Imported<n>& operator*=(const glm::mat<n, n, float>& m);
    Imported<n>* operator*(const glm::mat<n, n, float>& m) const;

    float* getPoints() const override;
    int getNumPoints() const override;

    void draw() override;
    void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
    void updateGLBuffers(GLenum usage = GL_STATIC_DRAW) override;

    GLenum glDrawMode() const override;

    void print() const override;
    void zoom(int percent) override;
    void rotate(int degrees) override;
    std::string fprint() const override;
};

#include "Imported.cpp"

#endif