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
        virtual float* getNormals() const override;
        virtual int getNumNormals() const override;

        virtual void print() const{
            std::cout << "_ P1 _ " << std::endl;
            std::cout << "TL: (" << tl[0];
            for(int i = 1; i < n; i++) std::cout << ", " << tl[i];
            std::cout << ")" << std::endl;
            std::cout << "_ P2 _ " << std::endl;
            std::cout << "TR: (" << tr[0];
            for(int i = 1; i < n; i++) std::cout << ", " << tr[i];
            std::cout << ")" << std::endl;
            std::cout << "_ P3 _ " <<std::endl;
            std::cout << "BR: (" << br[0];
            for(int i = 1; i < n; i++) std::cout << ", " << br[i];
            std::cout << ")" << std::endl;
            std::cout << "_ P4 _ " << std::endl;
            std::cout << "BL: (" << bl[0];
            for(int i = 1; i < n; i++) std::cout << ", " << bl[i];
            std::cout << ")" << std::endl;
        }
        virtual std::string fprint() const;
        void zoom(int percent);
        void rotate(int degrees);
        // GL draw mode
        virtual GLenum glDrawMode() const override;
        virtual void draw() override;
        virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
};

#include "Square.cpp"

#endif /*SQUARE_H*/
