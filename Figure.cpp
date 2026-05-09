#include "Figure.h"
#include <iostream>
#include <cmath>

Figure::Figure() : rotationAxisStart1({0.0f, 0.0f, 0.0f, 1.0f}), rotationAxisEnd1({0.0f, 0.0f, 1.0f, 1.0f}) {
}

Figure::~Figure() {
    for (auto& shape : shapes) {
        delete shape;
    }
    shapes.clear();
}

void Figure::addShape(Shape<4>* shape) {
    if (shape) {
        Shape3D* dumby=new Shape3D(shape);
        shapes.push_back(dumby);
    }
}

void Figure::addShape3D(Shape3D* shape3d) {
    if (shape3d) {
        shapes.push_back(shape3d);
    }
}

void Figure::rotate(int degrees) {


    Vector<3> rotationAxisStart=Vector<3>{rotationAxisStart1[0],rotationAxisStart1[1],rotationAxisStart1[2]};
    Vector<3> rotationAxisEnd=Vector<3>{rotationAxisEnd1[0],rotationAxisEnd1[1],rotationAxisEnd1[2]};
    Vector<3> V = rotationAxisEnd - rotationAxisStart;
    Vector<3> normalised;
    
    try {
        normalised = V.unitVector();
    } catch(const std::exception& e) {
        normalised = {0.0f, 0.0f, 1.0f};
    }
    
    // Convert degrees to radians
    float theta = degrees * 3.14159265359f / 180.0f;
    
    // Calculate d = sqrt(alpha_y^2 + alpha_z^2)
    float d = std::sqrt(normalised[1]*normalised[1] + normalised[2]*normalised[2]);
    
    // Build the rotation matrices according to the lecture notes
    // Rx(theta_x) matrix
    Matrix<4,4> Rx_1;
    if (d > 0.0f) {
        Rx_1[0][0] = 1.0f;  Rx_1[0][1] = 0.0f;              Rx_1[0][2] = 0.0f;              Rx_1[0][3] = 0.0f;
        Rx_1[1][0] = 0.0f;  Rx_1[1][1] = normalised[2]/d;   Rx_1[1][2] = -normalised[1]/d;  Rx_1[1][3] = 0.0f;
        Rx_1[2][0] = 0.0f;  Rx_1[2][1] = normalised[1]/d;   Rx_1[2][2] = normalised[2]/d;   Rx_1[2][3] = 0.0f;
        Rx_1[3][0] = 0.0f;  Rx_1[3][1] = 0.0f;              Rx_1[3][2] = 0.0f;              Rx_1[3][3] = 1.0f;
    } else {
        Rx_1 = Matrix<4,4>();  // Identity
    }
    
    // Ry(theta_y) matrix
    Matrix<4,4> Ry_1;
    Ry_1[0][0] = d;      Ry_1[0][1] = 0.0f;  Ry_1[0][2] = -normalised[0];     Ry_1[0][3] = 0.0f;
    Ry_1[1][0] = 0.0f;   Ry_1[1][1] = 1.0f;  Ry_1[1][2] = 0.0f;              Ry_1[1][3] = 0.0f;
    Ry_1[2][0] = normalised[0];     Ry_1[2][1] = 0.0f;  Ry_1[2][2] = d;       Ry_1[2][3] = 0.0f;
    Ry_1[3][0] = 0.0f;   Ry_1[3][1] = 0.0f;  Ry_1[3][2] = 0.0f;              Ry_1[3][3] = 1.0f;
    
    // Rz(theta) matrix
    float c = std::cos(theta);
    float s = std::sin(theta);
    Matrix<4,4> Rz;
    Rz[0][0] = c;   Rz[0][1] = -s;  Rz[0][2] = 0.0f;  Rz[0][3] = 0.0f;
    Rz[1][0] = s;   Rz[1][1] = c;   Rz[1][2] = 0.0f;  Rz[1][3] = 0.0f;
    Rz[2][0] = 0.0f;Rz[2][1] = 0.0f;Rz[2][2] = 1.0f;  Rz[2][3] = 0.0f;
    Rz[3][0] = 0.0f;Rz[3][1] = 0.0f;Rz[3][2] = 0.0f;  Rz[3][3] = 1.0f;
    
    // Ry(-theta_y) = inverse of Ry(theta_y)
    Matrix<4,4> Ry_inv = Ry_1;  // Transposed rotation is its inverse
    Ry_inv[0][2] = normalised[0];  // Flip sign
    Ry_inv[2][0] = -normalised[0];
    
    // Rx(-theta_x) = inverse of Rx(theta_x)
    Matrix<4,4> Rx_inv = Rx_1;
    if (d > 0.0f) {
        Rx_inv[1][2] = normalised[1]/d;  // Flip sign
        Rx_inv[2][1] = -normalised[1]/d;
    }
    
    // T(-P0) - using position as rotor center
    Matrix<4,4> T_neg = translation<4>(-position[0], -position[1], -position[2]);
    
    // T(P0)
    Matrix<4,4> T_pos = translation<4>(position[0], position[1], position[2]);
    
    // R = T(P0) * Rx(-θx) * Ry(-θy) * Rz(θ) * Ry(θy) * Rx(θx) * T(-P0)
    Matrix<4,4> finalRot = T_pos * Rx_inv * Ry_inv * Rz * Ry_1 * Rx_1 * T_neg;
    
    for (auto& shape : shapes) {
        *shape *= finalRot;
    }
}

void Figure::rotateX(int degrees) {

    Matrix<4,4> trans= rotx<4>(degrees);
    rotationAxisEnd1=Vector<4>(trans*((Matrix<4,1>)rotationAxisEnd1));
    rotationAxisStart1=Vector<4>(trans*((Matrix<4,1>)rotationAxisStart1));
    position =Vector<4>(trans*((Matrix<4,1>)position));

    for (auto& shape : shapes) {
        shape->rotateX(degrees);
    }
}

void Figure::rotateY(int degrees) {

    Matrix<4,4> trans= roty<4>(degrees);
    rotationAxisEnd1=Vector<4>(trans*((Matrix<4,1>)rotationAxisEnd1));
    rotationAxisStart1=Vector<4>(trans*((Matrix<4,1>)rotationAxisStart1));
    position =Vector<4>(trans*((Matrix<4,1>)position));

    for (auto& shape : shapes) {
        shape->rotateY(degrees);
    }
}

void Figure::rotateZ(int degrees) {


    Matrix<4,4> trans= rotz<4>(degrees);
    rotationAxisEnd1=Vector<4>(trans*((Matrix<4,1>)rotationAxisEnd1));
    rotationAxisStart1=Vector<4>(trans*((Matrix<4,1>)rotationAxisStart1));
    position =Vector<4>(trans*((Matrix<4,1>)position));

    for (auto& shape : shapes) {
        shape->rotateZ(degrees);
    }
}

void Figure::move(float x, float y, float z) {
    for (auto& shape : shapes) {
        shape->move(x, y, z);
    }
    Matrix<4,4> trans=translation<4>(x,y,z);
    rotationAxisEnd1=Vector<4>(trans*((Matrix<4,1>)rotationAxisEnd1));
    rotationAxisStart1=Vector<4>(trans*((Matrix<4,1>)rotationAxisStart1));
    position =Vector<4>(trans*((Matrix<4,1>)position));
}

void Figure::zoom(int percent) {
    for (auto& shape : shapes) {
        shape->zoom(percent);
    }
}

Figure& Figure::operator*=(const Matrix<4,4>& transform) {
    for (auto& shape : shapes) {
        *shape *= transform;
    }

    rotationAxisEnd1=Vector<4>(transform*((Matrix<4,1>)rotationAxisEnd1));
    rotationAxisStart1=Vector<4>(transform*((Matrix<4,1>)rotationAxisStart1));
    position =Vector<4>(transform*((Matrix<4,1>)position));
    return *this;
}

void Figure::draw(bool wireframe) {
    for (auto& shape : shapes) {
        shape->draw(wireframe);
    }
}

void Figure::setShapeColour(int r, int g, int b, float a) {
    for (auto& shape : shapes) {
        shape->setColour(r, g, b, a);
    }
}

int Figure::getNumShapes() const {
    return shapes.size();
}

Shape3D* Figure::getShape(int index) {
    if (index >= 0 && index < shapes.size()) {
        return shapes[index];
    }
    return nullptr;
}

void Figure::setPositionOffset(float x, float y, float z) {
    position = {x, y, z, 1.0f};
}

void Figure::setRotationAxis(float x1, float y1, float z1, float x2, float y2, float z2) {
    rotationAxisStart1 = {x1, y1, z1, 1.0f};
    rotationAxisEnd1 = {x2, y2, z2, 1.0f};
}

void Figure::createGLBuffers() {
    for (auto& shape : shapes) {
        shape->createGLBuffers();
    }
}

void Figure::updateGLBuffers() {
    for (auto& shape : shapes) {
        shape->updateGLBuffers();
    }
}



