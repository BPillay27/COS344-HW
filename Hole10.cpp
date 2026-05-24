#include "Hole10.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"

// Colours (0-255)
static const int TURF10_R = 34,  TURF10_G = 139, TURF10_B = 34;
static const int WALL10_R = 101, WALL10_G = 67,  WALL10_B = 33;
static const int CUP10_R  = 20,  CUP10_G  = 20,  CUP10_B  = 20;

static glm::vec4 pt10(float x, float y, float z, glm::vec3 off) {
    return glm::vec4(x + off.x, y + off.y, z + off.z, 1.0f);
}

void buildHole10(Figure& scene, glm::vec3 off) {
    const float HW = 3.5f;
    const float HOLE_LEN = 18.0f;
    const float WALL_T = 0.25f;
    const float WALL_H = 0.6f;
    const float MID_Z = -HOLE_LEN / 2.0f;
    
    // Base Turf
    Cube<4>* base = new Cube<4>(pt10(0.0f, 0.1f, MID_Z, off), 0.2f, HW, HOLE_LEN);
    base->setColour(TURF10_R, TURF10_G, TURF10_B);
    scene.addShape(base);

    // Left Border
    Cube<4>* leftW = new Cube<4>(pt10(-(HW/2.0f + WALL_T/2.0f), 0.3f, MID_Z, off), WALL_H, WALL_T, HOLE_LEN);
    leftW->setColour(WALL10_R, WALL10_G, WALL10_B);
    scene.addShape(leftW);

    // Right Border
    Cube<4>* rightW = new Cube<4>(pt10((HW/2.0f + WALL_T/2.0f), 0.3f, MID_Z, off), WALL_H, WALL_T, HOLE_LEN);
    rightW->setColour(WALL10_R, WALL10_G, WALL10_B);
    scene.addShape(rightW);

    // Start Cap
    Cube<4>* startW = new Cube<4>(pt10(0.0f, 0.3f, WALL_T/2.0f, off), WALL_H, HW+WALL_T*2.0f, WALL_T);
    startW->setColour(WALL10_R, WALL10_G, WALL10_B);
    scene.addShape(startW);

    // End Cap
    Cube<4>* endW = new Cube<4>(pt10(0.0f, 0.3f, -HOLE_LEN-WALL_T/2.0f, off), WALL_H, HW+WALL_T*2.0f, WALL_T);
    endW->setColour(WALL10_R, WALL10_G, WALL10_B);
    scene.addShape(endW);

    // Bottom Left Cup Position 
    glm::vec4 cupPos = pt10(-(HW/2.0f - 0.75f), 0.25f, -HOLE_LEN + 17.2f, off); 
    Cylinder<4>* cup = new Cylinder<4>(cupPos, 0.18f, 0.10f, 24, 1);
    cup->setColour(CUP10_R, CUP10_G, CUP10_B);
    scene.addShape(cup);

    // Deflectors (Rotated cubes to act as triangles against the walls)
    Figure* deflector1 = new Figure();
    Cube<4>* b1 = new Cube<4>(glm::vec4(0.0f, 0.3f, 0.0f, 1.0f), WALL_H, 2.0f, WALL_T);
    b1->setColour(WALL10_R, WALL10_G, WALL10_B); 
    deflector1->addShape(b1);
    deflector1->rotateY(45);
    deflector1->move(off.x - HW/2.0f + 0.5f, off.y, off.z - HOLE_LEN/3.0f);
    scene.addObject(deflector1);

    Figure* deflector2 = new Figure();
    Cube<4>* b2 = new Cube<4>(glm::vec4(0.0f, 0.3f, 0.0f, 1.0f), WALL_H, 2.0f, WALL_T);
    b2->setColour(WALL10_R, WALL10_G, WALL10_B);
    deflector2->addShape(b2);
    deflector2->rotateY(-45);
    deflector2->move(off.x + HW/2.0f - 0.5f, off.y, off.z - 2.0f*HOLE_LEN/3.0f);
    scene.addObject(deflector2);
}
