#pragma once
// Hole11.h
// Declaration for Hole 11 of the 19th Hole Putt-Putt course.
//
// Terrain: FLAT, CLIFF_DOWN, FLAT, RAMP_DOWN, FLAT
// Obstacle: two V-shaped concrete bumpers flanking the cup.
//
// Call buildHole11(scene, worldOffset) once during scene setup.
// worldOffset translates the entire hole into its position in world space.

#include <glm/glm.hpp>
#include "Figure.h"  // or whatever your scene container header is called

// rot180: when true the whole hole is rotated 180 degrees around the Y axis
//         (negates local X and Z before applying worldOffset).
void buildHole11(Figure& scene, glm::vec3 worldOffset, bool rot180 = false);