#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>

class Object {
protected:
    glm::vec4 position;
public:
    Object();
    virtual ~Object();

    // Basic transform/draw interface
    virtual void draw() = 0;
    virtual void move(float x, float y, float z)=0;
    virtual void rotate(int degrees)=0;
    virtual void rotateX(int degrees)=0;
    virtual void rotateY(int degrees)=0;
    virtual void rotateZ(int degrees)=0;
    virtual void zoom(int percent)=0;

    virtual void createGLBuffers()=0;
    virtual void updateGLBuffers()=0;
};

#endif // OBJECT_H
