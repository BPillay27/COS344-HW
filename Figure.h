#ifndef FIGURE_H
#define FIGURE_H

#include "Shape3D.h"
#include "Object.h"
#include <glm/glm.hpp>
#include <vector>

class Figure : public Object {
private:
    // CHANGED: Now holds base Object pointers to allow for true Composite structure
    // (i.e., can store Shape3D leaves OR other nested Figure branches).
    std::vector<Object*> children; 
    
    glm::vec4 rotationAxisStart1;  // First point defining rotation axis
    glm::vec4 rotationAxisEnd1;    // Second point defining rotation axis
    
public:
    Figure();
    virtual ~Figure(); // Added virtual destructor for safe polymorphic cleanup
    
    // --- Core Composite Methods ---
    void addObject(Object* obj);
    Object* getObject(int index) const;
    int getNumObjects() const;

    // --- Legacy Leaf Methods (Kept for backward compatibility e.g. LayoutReader) ---
    void addShape(Shape<4>* shape);
    void addShape3D(Shape3D* shape3d);
    int getNumShapes() const;
    Shape3D* getShape(int index) const;

    // --- Uniform Component Operations ---
    void rotate(int degrees);
    void rotateX(int degrees);
    void rotateY(int degrees);
    void rotateZ(int degrees);
    void move(float x, float y, float z);
    void zoom(int percent);
    Figure& operator*=(const glm::mat4& transform);
    void draw();
    
    // Kept original name, but internal logic is updated to cascade to components
    void setShapeColour(int r, int g, int b, float a);
    
    void setPositionOffset(float x, float y, float z);
    void setRotationAxis(float x1, float y1, float z1, float x2, float y2, float z2);
    void createGLBuffers();
    void updateGLBuffers();
};

#endif // FIGURE_H