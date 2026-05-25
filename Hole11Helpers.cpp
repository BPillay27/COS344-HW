// Hole11Helpers.cpp
// Helper functions shared within the Hole 11 build.
// Extracted so Hole11.cpp stays under 300 lines.

#include "Hole11Helpers.h"
#include "Shapes/Cube.h"
#include "Shapes/Square.h"

static const int H11_TURF_R     =  9, H11_TURF_G  = 139, H11_TURF_B  =  74;
static const int H11_WALL_R     = 101, H11_WALL_G  =  67, H11_WALL_B  =  33;
static const int H11_CONCRETE_R = 179, H11_CONCRETE_G = 177, H11_CONCRETE_B = 176;

// rot=true: 180-degree rotation around Y — negate local X and Z before offset.
glm::vec4 h11pt(float x, float y, float z, glm::vec3 o, bool rot) {
    if (rot) { x = -x; z = -z; }
    return glm::vec4(x + o.x, y + o.y, z + o.z, 1.0f);
}

void h11FlatTurf(Figure& scene, float cx, float cy, float cz,
                 float width, float depth, glm::vec3 o, bool rot) {
    Cube<4>* slab = new Cube<4>(h11pt(cx, cy, cz, o, rot), 0.2f, width, depth);
    slab->setColour(H11_TURF_R, H11_TURF_G, H11_TURF_B);
    scene.addShape(slab);
}

void h11SlopeTurf(Figure& scene,
                  float fx, float fy, float fz,
                  float bx, float by, float bz,
                  float width, glm::vec3 o, bool rot) {
    Square<4> front(h11pt(fx, fy, fz, o, rot), 0.2f, width);
    Square<4> back (h11pt(bx, by, bz, o, rot), 0.2f, width);
    Cube<4>* ramp = new Cube<4>(front, back);
    ramp->setColour(H11_TURF_R, H11_TURF_G, H11_TURF_B);
    scene.addShape(ramp);
}

void h11Wall(Figure& scene,
             float cx, float cy, float cz,
             float h, float w, float d, glm::vec3 o, bool rot) {
    Cube<4>* wall = new Cube<4>(h11pt(cx, cy, cz, o, rot), h, w, d);
    wall->setColour(H11_WALL_R, H11_WALL_G, H11_WALL_B);
    scene.addShape(wall);
}

void h11Concrete(Figure& scene,
                 float cx, float cy, float cz,
                 float h, float w, float d, glm::vec3 o, bool rot) {
    Cube<4>* block = new Cube<4>(h11pt(cx, cy, cz, o, rot), h, w, d);
    block->setColour(H11_CONCRETE_R, H11_CONCRETE_G, H11_CONCRETE_B);
    scene.addShape(block);
}

void h11VArm(Figure& scene,
             float tipX, float tipZ,
             float baseX, float baseZ,
             float surfaceY, glm::vec3 o, bool rot) {
    float armCY = surfaceY + 0.225f;
    Square<4> tipFace (h11pt(tipX,  armCY, tipZ,  o, rot), 0.45f, 0.15f);
    Square<4> baseFace(h11pt(baseX, armCY, baseZ, o, rot), 0.45f, 0.15f);
    Cube<4>*  arm = new Cube<4>(tipFace, baseFace);
    arm->setColour(H11_CONCRETE_R, H11_CONCRETE_G, H11_CONCRETE_B);
    scene.addShape(arm);
}
