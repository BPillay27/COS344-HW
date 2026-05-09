#ifndef SQUARE_H
#define SQUARE_H

#include <sstream>
#include <iomanip>
#include <cmath>

#include "Shape.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include "transformation.h"

template <int n>
class Square: public Shape<n> {
    private:
        glm::vec<n,float> tl;
        glm::vec<n,float> tr;
        glm::vec<n,float> br;
        glm::vec<n,float> bl;
    public:
        Square();
        Square(const glm::vec<n,float>& center, float height, float width);
        Square(const glm::vec<n,float>& tl, const glm::vec<n,float>& tr, const glm::vec<n,float>& br, const glm::vec<n,float>& bl);
        Square(const Square<n>&);
        virtual Square<n>& operator*=(const glm::mat<n,n,float>&);
        virtual Square<n>* operator*(const glm::mat<n,n,float>&) const;
        virtual float* getPoints() const;
        virtual int getNumPoints() const;

        virtual void print() const{
            std::cout << "_ P1 _ " << std::endl;
            tl.print();
            std::cout << "_ P2 _ " << std::endl;
            tr.print();
            std::cout << "_ P3 _ " <<std::endl;
            br.print();
            std::cout << "_ P4 _ " << std::endl;
            bl.print();
        }
        virtual std::string fprint() const;
        void zoom(int percent);
        void rotate(int degrees);
        // GL draw mode
        virtual GLenum glDrawMode() const override;
        virtual void draw(bool wireframe = false) override;
        virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
};

#include "Square.cpp"

#endif /*SQUARE_H*/
