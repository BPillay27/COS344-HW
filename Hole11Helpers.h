#pragma once
// Hole11Helpers.h
// Internal helper declarations for Hole11.cpp.
// Not intended for use by any other translation unit.

#include <glm/glm.hpp>
#include "Figure.h"

glm::vec4 h11pt(float x, float y, float z, glm::vec3 o);

void h11FlatTurf(Figure& scene, float cx, float cy, float cz,
                 float width, float depth, glm::vec3 o);

void h11SlopeTurf(Figure& scene,
                  float fx, float fy, float fz,
                  float bx, float by, float bz,
                  float width, glm::vec3 o);

void h11Wall(Figure& scene,
             float cx, float cy, float cz,
             float h, float w, float d, glm::vec3 o);

void h11Concrete(Figure& scene,
                 float cx, float cy, float cz,
                 float h, float w, float d, glm::vec3 o);

void h11VArm(Figure& scene,
             float tipX, float tipZ,
             float baseX, float baseZ,
             float surfaceY, glm::vec3 o);
