#ifndef CIRCLE_H
#define CIRCLE_H

#include <sstream>
#include <iomanip>
#include <cmath>

#include "Shape.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>

template <int n>
class Circle: public Shape<n> {
    private:
        glm::vec<n,float> center;
        float radius;
        int resolution; // number of segments for the fan
        float angleOffset = 0.0f; // radians offset for vertex sampling

    public:
        Circle();
        Circle(const glm::vec<n,float>& center, float radius, int resolution = 32);
        Circle(const Circle<n>&);
        virtual Circle<n>& operator*=(const glm::mat<n,n,float>&);
        virtual Circle<n>* operator*(const glm::mat<n,n,float>&) const;
        virtual float* getPoints() const;
        virtual int getNumPoints() const;

        virtual void print() const;
        virtual std::string fprint() const;
        void zoom(int percent);
        void rotate(int degrees);
        virtual GLenum glDrawMode() const override;
        virtual void draw(bool wireframe = false) override;
        virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
};

#include "Circle.cpp"

#endif /*CIRCLE_H*/