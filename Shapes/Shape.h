#ifndef SHAPE_H
#define SHAPE_H

#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <GL/glew.h>

template <int n>
class Shape{
    protected:
        float colour[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        GLuint VAO = 0u;
        GLuint VBO = 0u;
        GLuint EBO = 0u;
        // Separate buffers for optional per-vertex attributes
        GLuint texVBO = 0u;
        GLuint normVBO = 0u;
        GLuint colorVBO = 0u;
        int vertexCount = 0; // number of vertices (not floats)
    public:
        virtual Shape& operator*=(const glm::mat<n,n,float>&) =0;
        virtual Shape* operator*(const glm::mat<n,n,float>&) const =0;
        virtual float* getPoints() const =0;
        virtual int getNumPoints() const =0;

            virtual float* getTexCoords() const;
            virtual int getNumTexCoords() const;
            virtual float* getNormals() const;
            virtual int getNumNormals() const;
        virtual void print() const =0;
        virtual std::string fprint() const = 0;
        virtual void setColour(int r, int g, int b, float a=1.0f);
        float* getColour() const;
        virtual void zoom(int percent)=0;
        virtual void rotate(int degrees)=0;
        virtual void createGLBuffers(GLenum usage = GL_STATIC_DRAW);
        virtual void updateGLBuffers(GLenum usage = GL_DYNAMIC_DRAW);
        // Set per-vertex colors (RGB floats, numFloats must be verts*3)
        void setPerVertexColors(const float* colors, int numFloats, GLenum usage = GL_DYNAMIC_DRAW);
        void deleteGLBuffers();
        virtual GLenum glDrawMode() const = 0;
        virtual void draw() = 0;
        Shape() = default;
            // Safe copy/move semantics: copies do NOT duplicate GL handles.
        Shape(const Shape& other);
        Shape& operator=(const Shape& other);
        Shape(Shape&& other) noexcept;
        Shape& operator=(Shape&& other) noexcept;
        virtual ~Shape();
};
#include "Shape.cpp"

#endif /*SHAPE_H*/