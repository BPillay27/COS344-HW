// Hole8.cpp
// Constructs all geometry for Hole 8 of the 19th Hole Putt-Putt course.
//
// Hole 8 description (from the final report / hole list):
//   Terrain   : FLAT, BUMP, FLAT, BUMP, FLAT
//               Two bumps – each has a rise, a flat top, and a fall.
//               The golf cup sits on the final flat section after bump 2.
//   Obstacle  : brick border walls (the sole obstacle on this hole).
//
// Local coordinate system (worldOffset shifts everything into scene space):
//   X  = width  (hole is 3.5 units wide, centred on x=0)
//   Y  = height (up)
//   Z  = length (tee at z=0, cup at z=-15.5, end wall at z=-16.5)
//
// Terrain cross-section (z left->right, y up):
//            bump1 top        bump2 top
//          ____________      ____________
//         /            \    /            \
// -------/              \--/              \-----------  (cup here)
// z=0  -2  -3.5         -6-7.5 -9.5  -11.5 -13 -14.5  -16.5

#include "Hole8.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Square.h"

// Colours (0-255 integers)
static const int TURF_R = 34,  TURF_G = 139, TURF_B = 34;   // dark green turf
static const int WALL_R = 101, WALL_G = 67,  WALL_B = 33;   // wooden border walls
static const int CUP_R  = 20,  CUP_G  = 20,  CUP_B  = 20;  // near-black cup

// Helper: build a vec4 with world offset applied.
static glm::vec4 pt(float x, float y, float z, glm::vec3 off) {
    return glm::vec4(x + off.x, y + off.y, z + off.z, 1.0f);
}

// Helper: axis-aligned turf slab (0.2 units thick in Y).
static void addFlatTurf(Figure& scene,
                        float cx, float cy, float cz,
                        float width, float depth,
                        glm::vec3 off) {
    Cube<4>* slab = new Cube<4>(pt(cx, cy, cz, off), 0.2f, width, depth);
    slab->setColour(TURF_R, TURF_G, TURF_B);
    scene.addShape(slab);
}

// Helper: sloped turf slab connecting two face centres at different Y heights.
static void addSlopeTurf(Figure& scene,
                         float fx, float fy, float fz,
                         float bx, float by, float bz,
                         float width,
                         glm::vec3 off) {
    Square<4> frontFace(pt(fx, fy, fz, off), 0.2f, width);
    Square<4> backFace (pt(bx, by, bz, off), 0.2f, width);
    Cube<4>* slope = new Cube<4>(frontFace, backFace);
    slope->setColour(TURF_R, TURF_G, TURF_B);
    scene.addShape(slope);
}

// Helper: border wall slab.
static void addWall(Figure& scene,
                    float cx, float cy, float cz,
                    float h, float w, float d,
                    glm::vec3 off) {
    Cube<4>* wall = new Cube<4>(pt(cx, cy, cz, off), h, w, d);
    wall->setColour(WALL_R, WALL_G, WALL_B);
    scene.addShape(wall);
}

// -----------------------------------------------------------------
// buildHole8: assembles every piece of Hole 8 into the scene Figure.
// -----------------------------------------------------------------
void buildHole8(Figure& scene, glm::vec3 off) {
    const float HW = 3.5f;  // hole width (X extent)

    // ============================================================
    // TERRAIN: 9 sections -> FLAT, BUMP, FLAT, BUMP, FLAT
    // ============================================================

    // Section 1: start flat (z=0 to -2)
    addFlatTurf(scene, 0.0f, 0.1f, -1.0f, HW, 2.0f, off);

    // Section 2: bump 1 rise (z=-2 to -3.5)
    addSlopeTurf(scene, 0.0f, 0.1f, -2.0f,
                        0.0f, 0.9f, -3.5f, HW, off);

    // Section 3: bump 1 flat top (z=-3.5 to -6, terrain top y=1.0)
    addFlatTurf(scene, 0.0f, 0.9f, -4.75f, HW, 2.5f, off);

    // Section 4: bump 1 fall (z=-6 to -7.5)
    addSlopeTurf(scene, 0.0f, 0.9f, -6.0f,
                        0.0f, 0.1f, -7.5f, HW, off);

    // Section 5: mid flat (z=-7.5 to -9.5)
    addFlatTurf(scene, 0.0f, 0.1f, -8.5f, HW, 2.0f, off);

    // Section 6: bump 2 rise (z=-9.5 to -11.5)
    addSlopeTurf(scene, 0.0f, 0.1f,  -9.5f,
                        0.0f, 1.3f, -11.5f, HW, off);

    // Section 7: bump 2 flat top (z=-11.5 to -13, terrain top y=1.4)
    addFlatTurf(scene, 0.0f, 1.3f, -12.25f, HW, 1.5f, off);

    // Section 8: bump 2 fall (z=-13 to -14.5)
    addSlopeTurf(scene, 0.0f, 1.3f, -13.0f,
                        0.0f, 0.1f, -14.5f, HW, off);

    // Section 9: end flat (z=-14.5 to -16.5, cup here)
    addFlatTurf(scene, 0.0f, 0.1f, -15.5f, HW, 2.0f, off);

    // ============================================================
    // BORDER WALLS
    // Base perimeter runs the full lane length at y=0 to y=0.6.
    // Extra raised slabs close the open gaps on each bump's sides
    // where the terrain rises above y=0.6.
    // ============================================================
    const float HOLE_LEN = 16.5f;
    const float MID_Z    = -HOLE_LEN * 0.5f;   // z centre = -8.25
    const float WALL_H   = 0.6f;
    const float WALL_T   = 0.25f;
    const float LX       = HW * 0.5f + WALL_T * 0.5f;  // x centre of side walls = 1.875

    // Base perimeter (y = 0 to 0.6, full length)
    addWall(scene, -LX,  0.3f, MID_Z, WALL_H, WALL_T, HOLE_LEN, off);  // left
    addWall(scene,  LX,  0.3f, MID_Z, WALL_H, WALL_T, HOLE_LEN, off);  // right
    addWall(scene, 0.0f, 0.3f,  WALL_T * 0.5f,            WALL_H, HW + WALL_T * 2.0f, WALL_T, off);  // start cap
    addWall(scene, 0.0f, 0.3f, -HOLE_LEN - WALL_T * 0.5f, WALL_H, HW + WALL_T * 2.0f, WALL_T, off);  // end cap

    // Extra walls: bump 1 (terrain top = y 1.0)
    // Rise  z=-2 to -3.5  (depth=1.5, cz=-2.75) extra covers y=0.6 to 1.2 (h=0.6, cy=0.9)
    addWall(scene, -LX, 0.9f, -2.75f, WALL_H, WALL_T, 1.5f, off);
    addWall(scene,  LX, 0.9f, -2.75f, WALL_H, WALL_T, 1.5f, off);
    // Flat top  z=-3.5 to -6  (depth=2.5, cz=-4.75) extra covers y=0.6 to 1.6 (h=1.0, cy=1.1)
    addWall(scene, -LX, 1.1f, -4.75f, 1.0f, WALL_T, 2.5f, off);
    addWall(scene,  LX, 1.1f, -4.75f, 1.0f, WALL_T, 2.5f, off);
    // Fall  z=-6 to -7.5  (depth=1.5, cz=-6.75) extra covers y=0.6 to 1.2 (h=0.6, cy=0.9)
    addWall(scene, -LX, 0.9f, -6.75f, WALL_H, WALL_T, 1.5f, off);
    addWall(scene,  LX, 0.9f, -6.75f, WALL_H, WALL_T, 1.5f, off);

    // Extra walls: bump 2 (terrain top = y 1.4)
    // Rise  z=-9.5 to -11.5  (depth=2.0, cz=-10.5) extra covers y=0.6 to 1.4 (h=0.8, cy=1.0)
    addWall(scene, -LX, 1.0f, -10.5f, 0.8f, WALL_T, 2.0f, off);
    addWall(scene,  LX, 1.0f, -10.5f, 0.8f, WALL_T, 2.0f, off);
    // Flat top  z=-11.5 to -13  (depth=1.5, cz=-12.25) extra covers y=0.6 to 2.0 (h=1.4, cy=1.3)
    addWall(scene, -LX, 1.3f, -12.25f, 1.4f, WALL_T, 1.5f, off);
    addWall(scene,  LX, 1.3f, -12.25f, 1.4f, WALL_T, 1.5f, off);
    // Fall  z=-13 to -14.5  (depth=1.5, cz=-13.75) extra covers y=0.6 to 1.4 (h=0.8, cy=1.0)
    addWall(scene, -LX, 1.0f, -13.75f, 0.8f, WALL_T, 1.5f, off);
    addWall(scene,  LX, 1.0f, -13.75f, 0.8f, WALL_T, 1.5f, off);

    // ============================================================
    // GOLF CUP - end flat at z=-15.5, just proud of the turf surface
    // ============================================================
    {
        glm::vec4 cupCentre = pt(0.0f, 0.25f, -15.5f, off);
        Cylinder<4>* cup = new Cylinder<4>(cupCentre, 0.18f, 0.15f, 24, 1);
        cup->setColour(CUP_R, CUP_G, CUP_B);
        scene.addShape(cup);
    }
}
