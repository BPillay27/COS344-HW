#ifndef CONE_H
#define CONE_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream>
#include <iomanip>

#include "Shape.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>

template <int n>
class Cone: public Shape<n> {
    private:
        glm::vec<n,float> apex;        // tip of the cone
        glm::vec<n,float> baseCenter;  // centre of the circular base
        float radius;
        float height;
        int resolution;
        float angleOffset = 0.0f;
        int axis = 2; // 0=x,1=y,2=z

    public:
        Cone();
        // base center + height along axis
        Cone(const glm::vec<n,float>& baseCenter, float radius, float height, int resolution = 32, int axis = 2);
        // explicit apex and base centre
        Cone(const glm::vec<n,float>& apex, const glm::vec<n,float>& baseCenter, float radius, int resolution = 32);
        Cone(const Cone<n>&);

        virtual Cone<n>& operator*=(const glm::mat<n,n,float>&);
        virtual Cone<n>* operator*(const glm::mat<n,n,float>&) const;
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

#include "Cone.cpp"

#endif /*CONE_H*/
