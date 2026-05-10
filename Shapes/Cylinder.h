#ifndef CYLINDER_H
#define CYLINDER_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream>
#include <iomanip>

#include "Shape.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>

template <int n>
class Cylinder: public Shape<n> {
    private:
        glm::vec<n,float> center;
        float radius;
        float height;
        int resolution; // number of segments around the circle
        float angleOffset = 0.0f; // radians offset for sampling
        int axis = 2; // 0 = x, 1 = y, 2 = z (which axis the cylinder's central axis runs along)
        glm::vec<n,float> basisU;  // First perpendicular basis vector for circular cross-section
        glm::vec<n,float> basisV;  // Second perpendicular basis vector for circular cross-section

    public:
        Cylinder();
        Cylinder(const glm::vec<n,float>& center, float radius, float height, int resolution = 32, int axis = 2);
        // Construct from two circle centres (c1 and c2 are centres of the two circular faces)
        Cylinder(const glm::vec<n,float>& c1, const glm::vec<n,float>& c2, float radius, int resolution = 32);
        Cylinder(const Cylinder<n>&);
        virtual Cylinder<n>& operator*=(const glm::mat<n,n,float>&);
        virtual Cylinder<n>* operator*(const glm::mat<n,n,float>&) const;
        virtual float* getPoints() const;
        virtual int getNumPoints() const;
        virtual float* getTexCoords() const;
        virtual int getNumTexCoords() const;
        virtual float* getNormals() const;
        virtual int getNumNormals() const;
        // Normal helpers
        glm::vec<n,float> normalAtAngle(float angle) const; // normal for side at given angle (radians)
        glm::vec<n,float> normalAtPoint(const glm::vec<n,float>& p) const; // normal for arbitrary point on cylinder

        virtual void print() const;
        virtual std::string fprint() const;
        void zoom(int percent);
        void rotate(int degrees);
        // Change resolution (number of segments around the circle)
        void setResolution(int r);
        int getResolution() const;
        virtual GLenum glDrawMode() const override;
        virtual void draw() override;
        virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW) override;
};

#include "Cylinder.cpp"

#endif /*CYLINDER_H*/
