#ifndef CUBE_H
#define CUBE_H

#include <sstream>
#include <iomanip>
#include <cmath>

#include "Shape.h"
#include "Square.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include "transformation.h"

template <int n>
class Cube: public Shape<n> {
    private:
        Square<n> f1;
        Square<n> f2;
        Square<n> f3;
        Square<n> f4;
        Square<n> f5;
        Square<n> f6;
        glm::vec<n,float> center;

    public:
        Cube();
        Cube(const glm::vec<n,float>& center, float height, float width, float depth);
        Cube(const Square<n>& f1, const Square<n>& f2); // It determines it's own centre
        Cube(const Cube<n>&);
        virtual Cube<n>& operator*=(const glm::mat<n,n,float>&);
        virtual Cube<n>* operator*(const glm::mat<n,n,float>&) const;
        virtual float* getPoints() const;
        virtual int getNumPoints() const;

        virtual void print() const;
        virtual std::string fprint() const;
        void zoom(int percent);
        void rotate(int degrees);
        virtual GLenum glDrawMode() const override;
        virtual void draw(bool wireframe = false) override;
        virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
        virtual void updateGLBuffers(GLenum usage = GL_DYNAMIC_DRAW) override;
        void setColour(int r, int g, int b, float a=1.0f);
};

#include "Cube.cpp"

#endif /*CUBE_H*/