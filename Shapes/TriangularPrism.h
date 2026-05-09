#ifndef TRIANGULARPRISM_H
#define TRIANGULARPRISM_H

#include "Shape.h"
#include "Triangle.h"
#include "Square.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>

template<int n>
class TriangularPrism: public Shape<n>{
private:
    Triangle<n> triangle1;
    Triangle<n> triangle2;
    Square<n> side1;
    Square<n> side2;
    Square<n> side3;

public:
    TriangularPrism();
    TriangularPrism(const Triangle<n>& t1, const Triangle<n>& t2, const Square<n>& s1, const Square<n>& s2, const Square<n>& s3);
    TriangularPrism(const Triangle<n>& t1, const Triangle<n>& t2);
    TriangularPrism(const TriangularPrism<n>& other);
    
    TriangularPrism<n>& operator*=(const glm::mat<n,n,float>& m);
    TriangularPrism<n>* operator*(const glm::mat<n,n,float>& m) const;
    
    float* getPoints() const;
    int getNumPoints() const;
    void draw(bool wireframe);
    void createGLBuffers(GLenum usage = GL_DYNAMIC_DRAW);
    
    void print() const;
    std::string fprint() const;
    void zoom(int percent);
    void rotate(int degrees);
};

#include "TriangularPrism.cpp"
#endif
