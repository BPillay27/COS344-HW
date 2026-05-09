#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include "Shape.h"
#include "transformation.h"

template<int n>
class Sphere : public Shape<n> {
public:
    glm::vec<n,float> center;
    float radius = 1.0f;
    int stacks = 16; // latitude
    int slices = 32; // longitude

    Sphere();
    Sphere(const glm::vec<n,float>& center, float radius, int stacks = 16, int slices = 32);
    Sphere(const Sphere& other);
    Sphere& operator*=(const glm::mat<n,n,float>& m) override;
    Sphere* operator*(const glm::mat<n,n,float>& m) const override;
    float* getPoints() const override;
    int getNumPoints() const override;
    float* getTexCoords() const override;
    int getNumTexCoords() const override;
    float* getNormals() const override;
    int getNumNormals() const override;
    // Normal helpers
    glm::vec<n,float> normalAtParams(float phi, float theta) const; // phi: latitude, theta: longitude
    glm::vec<n,float> normalAtPoint(const glm::vec<n,float>& p) const; // normal for given point on sphere surface
    void print() const override;
    std::string fprint() const override;
    void zoom(int percent) override;
    void rotate(int degrees) override;
    void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
    void updateGLBuffers(GLenum usage = GL_DYNAMIC_DRAW) override;
    GLenum glDrawMode() const override;
    void draw(bool wireframe = false) override;
private:
    void pushVertex(float* result, int &idx, float phi, float theta) const;
};

#include "Sphere.cpp"

#endif // SPHERE_H
