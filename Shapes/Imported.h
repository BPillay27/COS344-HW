#ifndef IMPORTED_H
#define IMPORTED_H

#include "Shape.h"
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>

template<int n>
class Imported : public Shape<n> {
private:
    struct Mesh {
        std::vector<glm::vec<n,float>> vertices;
        std::vector<GLuint> indices;
        std::vector<glm::vec3> normals;
    };
    
    std::vector<Mesh> meshes;
    std::string filepath;
    
    void loadGLTF(const std::string& filePath);
    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh);
    
public:
    Imported();
    explicit Imported(const std::string& filePath);
    Imported(const Imported<n>& other);
    
    virtual Imported<n>& operator*=(const glm::mat<n,n,float>& m);
    virtual Imported<n>* operator*(const glm::mat<n,n,float>& m) const;
    
    virtual float* getPoints() const;
    virtual int getNumPoints() const;
    virtual void draw();
    virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW);
    virtual void updateGLBuffers(GLenum usage = GL_DYNAMIC_DRAW);
    virtual GLenum glDrawMode() const;
    
    virtual void print() const;
    virtual void zoom(int percent);
    virtual void rotate(int degrees);
    virtual std::string fprint() const;
    
    std::vector<Mesh>& getMeshes() { return meshes; }
    const std::vector<Mesh>& getMeshes() const { return meshes; }
};

#include "Imported.cpp"
#endif // IMPORTED_H
