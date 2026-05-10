#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <sstream>
#include <iomanip>
#include <cmath>

#include "Shape.h"
#include <glm/glm.hpp>
#include "transformation.h"

template <int n>
class Triangle: public Shape<n> {
    private:
        glm::vec<n,float> p1;
        glm::vec<n,float> p2;
        glm::vec<n,float> p3;
    public:
        Triangle();
        Triangle(const glm::vec<n,float>& p1, const glm::vec<n,float>& p2, const glm::vec<n,float>& p3);
        Triangle(const Triangle<n>&);
        virtual Triangle<n>& operator*=(const glm::mat<n,n,float>&);
        virtual Triangle<n>* operator*(const glm::mat<n,n,float>&) const;
        virtual float* getPoints() const;
        virtual int getNumPoints() const;

        virtual void print() const{
            std::cout << "_ P1 _ " << std::endl;
            p1.print();
            std::cout << "_ P2 _ " << std::endl;
            p2.print();
            std::cout << "_ P3 _ " << std::endl;
            p3.print();
        }
        virtual std::string fprint() const;
        void zoom(int percent);
        void rotate(int degrees);
        virtual GLenum glDrawMode() const override;
        virtual void draw() override;
        virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
};

#include "Triangle.cpp"

#endif /*TRIANGLE_H*/
