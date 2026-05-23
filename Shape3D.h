#ifndef SHAPE3D_H
#define SHAPE3D_H

#include "Shapes/Shape.h"
#include "Shapes/transformation.h"
#include <glm/glm.hpp>
#include "Object.h"

class Shape3D : public Object {
private:
    Shape<4>* shape; // owning pointer; this class owns and deletes the shape
    
public:
    explicit Shape3D(Shape<4>* s);
    Shape3D(const Shape3D& other);  // deep copy: creates new owned shape
    Shape3D& operator=(const Shape3D& other);  // deep copy assignment
    Shape3D& operator*=(const glm::mat4& transform);
    Shape3D* operator*(const glm::mat4& transform) const;
    
    void rotate(int degrees);
    void rotateX(int degrees);
    void rotateY(int degrees);
    void rotateZ(int degrees);
    void move(float x, float y, float z);
    void zoom(int percent);
    void draw();
    void setColour(int r, int g, int b, float a);
    Shape<4>* getShape() const;
    
    void createGLBuffers();
    void updateGLBuffers();
    
    virtual ~Shape3D();
};

#endif // SHAPE3D_H
