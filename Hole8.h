#ifndef HOLE8_H
#define HOLE8_H

#include "Figure.h"
#include <glm/glm.hpp>

// Builds all geometry for Hole 8 and adds every shape to the given scene Figure.
// Call this before scene.createGLBuffers().
// worldOffset positions the hole's local origin (z=0, start pad) in scene space.
void buildHole8(Figure& scene, glm::vec3 worldOffset);

#endif // HOLE8_H
