#ifndef SQUAREPYRAMID_H
#define SQUAREPYRAMID_H

#include "Shape.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include "Square.h"
#include "Triangle.h"
#include <GL/glew.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

template <int n>
class SquarePyramid : public Shape<n> {
private:
    Square<n> base;
    Triangle<n> sides[4];

public:
    SquarePyramid();
    SquarePyramid(const glm::vec<n,float>& baseCenter, float baseSize, float height, int axis);
    SquarePyramid(const glm::vec<n,float>& apex, const glm::vec<n,float>& baseCenter, float baseSize);
    SquarePyramid(const Square<n>& base, const glm::vec<n,float>& apex);
    SquarePyramid(const SquarePyramid<n>& other);
    
    virtual SquarePyramid<n>& operator*=(const glm::mat<n,n,float>& m);
    virtual SquarePyramid<n>* operator*(const glm::mat<n,n,float>& m) const;
    
    virtual float* getPoints() const;
    virtual int getNumPoints() const;
    virtual void draw(bool wireframe = false);
    virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW);
    virtual void updateGLBuffers(GLenum usage = GL_DYNAMIC_DRAW);
    virtual GLenum glDrawMode() const;
    
    virtual void print() const;
    virtual void zoom(int percent);
    virtual void rotate(int degrees);
    virtual std::string fprint() const;
    void setColour(int r, int g, int b, float a=1.0f);
};

#include "SquarePyramid.cpp"
#endif
