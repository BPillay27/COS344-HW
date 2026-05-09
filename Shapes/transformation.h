#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/glm.hpp>

template<int n>
glm::mat<n,n,float> roty(int m);

template<int n>
glm::mat<n,n,float> rotx(int m);

template<int n>
glm::mat<n,n,float> rotz(int m);

template<int n>
glm::mat<n,n,float> shearx(int xy,int xz);

template<int n>
glm::mat<n,n,float> sheary(int yx,int yz);

template<int n>
glm::mat<n,n,float> shearz(int zx,int zy);

template<int n>
glm::mat<n,n,float> translation(float x,float y,float z);

template<int n>
glm::mat<n,n,float> scaling(float m);

#include "transformation.cpp"

#endif //TRANSFORMATION_H