#include "Shape3D.h"
Shape3D::Shape3D(Shape<4>* s) : shape(s) {
    // Object base initializes `position` to default
}

Shape3D::Shape3D(const Shape3D& other) 
    : shape(other.shape) {
    // shallow copy: do not allocate or own the underlying Shape
    position = other.position;
}

Shape3D& Shape3D::operator=(const Shape3D& other) {
    if (this != &other) {
        // shallow assignment: do not delete or take ownership
        shape = other.shape;
        position = other.position;
    }
    return *this;
}

Shape3D& Shape3D::operator*=(const glm::mat4& transform) {
    if (shape) {
        *shape *= transform;
        shape->updateGLBuffers();
    }
    return *this;
}

Shape3D* Shape3D::operator*(const glm::mat4& transform) const {
    if (shape) {
        // produce a Shape3D that references a newly-allocated Shape;
        // Note: this class does not take ownership, so caller must manage
        // the underlying Shape memory if needed.
        Shape3D* result = new Shape3D(shape->operator*(transform));
        result->position = position;
        return result;
    }
    return nullptr;
}

void Shape3D::rotate(int degrees) {
    if (shape) {
        glm::mat4 trans = rotz<4>(degrees);
        *shape *= trans;
        shape->updateGLBuffers();
    }
}

void Shape3D::rotateX(int degrees) {
    if (shape) {
        glm::mat4 trans = rotx<4>(degrees);
        *shape *= trans;
        shape->updateGLBuffers();
    }
}

void Shape3D::rotateY(int degrees) {
    if (shape) {
        glm::mat4 trans = roty<4>(degrees);
        *shape *= trans;
        shape->updateGLBuffers();
    }
}

void Shape3D::rotateZ(int degrees) {
    if (shape) {
        glm::mat4 trans = rotz<4>(degrees);
        *shape *= trans;
        shape->updateGLBuffers();
    }
}

void Shape3D::move(float x, float y, float z) {
    glm::mat4 trans = translation<4>(x, y, z);
    if (shape) {
        *shape *= trans;
        shape->updateGLBuffers();
    }
    position = glm::vec4(trans * glm::vec4(position));
}

void Shape3D::zoom(int percent) {
    if (shape) {
        shape->zoom(percent);
        shape->updateGLBuffers();
    }
}

void Shape3D::draw(bool wireframe) {
    if (shape) {
        shape->draw(wireframe);
    }
}

void Shape3D::setColour(int r, int g, int b, float a) {
    if (shape) {
        shape->setColour(r, g, b, a);
    }
}

Shape<4>* Shape3D::getShape() const {
    return shape;
}

void Shape3D::createGLBuffers() {
    if (shape) {
        shape->createGLBuffers();
    }
}

void Shape3D::updateGLBuffers() {
    if (shape) {
        shape->updateGLBuffers();
    }
}

Shape3D::~Shape3D() {
    // non-owning: do not delete `shape` here
}
