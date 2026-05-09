#ifndef SHAPE3D_H
#define SHAPE3D_H

#include "shapes/Shape.h"
#include "shapes/transformation.h"
#include <glm/glm.hpp>
#include "Object.h"

class Shape3D : public Object {
private:
    Shape<4>* shape; // non-owning raw pointer; this class does NOT delete it    
public:
    explicit Shape3D(Shape<4>* s);
    Shape3D(const Shape3D& other);
    Shape3D& operator=(const Shape3D& other);
    Shape3D& operator*=(const glm::mat4& transform);
    Shape3D* operator*(const glm::mat4& transform) const;
    
    void rotate(int degrees);
    void rotateX(int degrees);
    void rotateY(int degrees);
    void rotateZ(int degrees);
    void move(float x, float y, float z);
    void zoom(int percent);
    void draw(bool wireframe = false);
    void setColour(int r, int g, int b, float a);
    Shape<4>* getShape() const;
    
    void createGLBuffers();
    void updateGLBuffers();
    
    virtual ~Shape3D();
};

#endif // SHAPE3D_H
