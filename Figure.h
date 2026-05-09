#ifndef FIGURE_H
#define FIGURE_H

#include "Shape3D.h"
#include "Object.h"
#include "shapes/Matrix.h"
#include "shapes/Vector.h"
#include <vector>


class Figure : public Object {
private:
    std::vector<Shape3D* > shapes;
    Vector<4> rotationAxisStart1;  // First point defining rotation axis
    Vector<4> rotationAxisEnd1;    // Second point defining rotation axis
    
public:
    Figure();
    ~Figure();
    
    void addShape(Shape<4>* shape);
    void addShape3D(Shape3D* shape3d);
    void rotate(int degrees);
    void rotateX(int degrees);
    void rotateY(int degrees);
    void rotateZ(int degrees);
    void move(float x, float y, float z);
    void zoom(int percent);
    Figure& operator*=(const Matrix<4,4>& transform);
    void draw(bool wireframe = false);
    void setShapeColour(int r, int g, int b, float a);
    int getNumShapes() const;
    Shape3D* getShape(int index);
    void setPositionOffset(float x, float y, float z);
    void setRotationAxis(float x1, float y1, float z1, float x2, float y2, float z2);
    void createGLBuffers();
    void updateGLBuffers();
};

#endif // FIGURE_H
