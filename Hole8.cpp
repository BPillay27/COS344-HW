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
//   Z  = length (tee at z=0, cup at z≈-15.5, end wall at z=-16.5)
//
// Terrain cross-section (z axis left→right, y axis up):
//
//            bump1 top        bump2 top
//          ____________      ____________
//         /            \    /            \
// ───────/              \──/              \───────────  (cup here)
// z=0  -2  -3.5         -6-7.5 -9.5   -11.5  -13  -14.5  -16.5

#include "Hole8.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Square.h"

// Colours (0–255, matching the setColour convention used throughout the project)
static const int TURF_R = 34,  TURF_G = 139, TURF_B = 34;   // dark green turf
static const int WALL_R = 101, WALL_G = 67,  WALL_B = 33;   // wooden border walls
static const int CUP_R  = 20,  CUP_G  = 20,  CUP_B  = 20;  // near-black cup
static const int POLE_R = 200, POLE_G = 200, POLE_B = 200;  // light grey flag pole
static const int FLAG_R = 220, FLAG_G = 30,  FLAG_B = 30;   // red flag

// Helper: build a vec4 from local floats plus the world offset.
static glm::vec4 pt(float x, float y, float z, glm::vec3 off) {
    return glm::vec4(x + off.x, y + off.y, z + off.z, 1.0f);
}

// Helper: add an axis-aligned turf slab (0.2 units thick in Y).
//   cx, cy, cz = centre of the slab
//   width = X extent, depth = Z extent
static void addFlatTurf(Figure& scene,
                        float cx, float cy, float cz,
                        float width, float depth,
                        glm::vec3 off) {
    Cube<4>* slab = new Cube<4>(pt(cx, cy, cz, off), 0.2f, width, depth);
    slab->setColour(TURF_R, TURF_G, TURF_B);
    scene.addShape(slab);
}

// Helper: add a sloped turf slab connecting two parallel face centres.
//   The front face centre is (fx,fy,fz) and the back face centre is (bx,by,bz).
//   Both faces are 0.2 units tall (Y) and `width` units wide (X).
//   Connecting them with Cube(Square,Square) produces a parallelogram prism,
//   which reads as a slope when the two centres differ in Y.
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

// Helper: add a border wall slab.
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
    // TERRAIN: 9 sections → FLAT, BUMP, FLAT, BUMP, FLAT
    //
    // z range    section          y-centre (slab midpoint)
    // ─────────────────────────────────────────────────────────
    //  0  → -2   start flat        0.1
    // -2  → -3.5 bump1 rise        0.1 (front) → 0.9 (back)
    // -3.5→ -6   bump1 top         0.9
    // -6  → -7.5 bump1 fall        0.9 (front) → 0.1 (back)
    // -7.5→ -9.5 mid flat          0.1
    // -9.5→-11.5 bump2 rise        0.1 (front) → 1.3 (back)
    //-11.5→-13   bump2 top         1.3
    //-13 →-14.5  bump2 fall        1.3 (front) → 0.1 (back)
    //-14.5→-16.5 end flat (cup)    0.1
    // ============================================================

    // Section 1: start flat
    addFlatTurf(scene, 0.0f, 0.1f, -1.0f, HW, 2.0f, off);

    // Section 2: bump 1 rise
    addSlopeTurf(scene,
                 0.0f, 0.1f, -2.0f,
                 0.0f, 0.9f, -3.5f,
                 HW, off);

    // Section 3: bump 1 flat top
    addFlatTurf(scene, 0.0f, 0.9f, -4.75f, HW, 2.5f, off);

    // Section 4: bump 1 fall
    addSlopeTurf(scene,
                 0.0f, 0.9f, -6.0f,
                 0.0f, 0.1f, -7.5f,
                 HW, off);

    // Section 5: mid flat
    addFlatTurf(scene, 0.0f, 0.1f, -8.5f, HW, 2.0f, off);

    // Section 6: bump 2 rise
    addSlopeTurf(scene,
                 0.0f, 0.1f,  -9.5f,
                 0.0f, 1.3f, -11.5f,
                 HW, off);

    // Section 7: bump 2 flat top
    addFlatTurf(scene, 0.0f, 1.3f, -12.25f, HW, 1.5f, off);

    // Section 8: bump 2 fall  ← previously missing
    addSlopeTurf(scene,
                 0.0f, 1.3f, -13.0f,
                 0.0f, 0.1f, -14.5f,
                 HW, off);

    // Section 9: end flat (cup sits here)
    addFlatTurf(scene, 0.0f, 0.1f, -15.5f, HW, 2.0f, off);

    // ============================================================
    // BORDER WALLS (the sole obstacle on Hole 8).
    // Four slabs surround the full perimeter of the 16.5-unit lane.
    // Height 0.6 keeps them visible against the tallest bump (y=1.4).
    // ============================================================
    const float HOLE_LEN = 16.5f;
    const float MID_Z    = -HOLE_LEN * 0.5f;  // z centre of the lane = -8.25
    const float WALL_H   = 0.6f;
    const float WALL_T   = 0.25f;             // wall thickness

    // Left side wall  (x = -HW/2 - WALL_T/2)
    addWall(scene, -(HW * 0.5f + WALL_T * 0.5f), 0.3f, MID_Z, WALL_H, WALL_T, HOLE_LEN, off);
    // Right side wall (x = +HW/2 + WALL_T/2)
    addWall(scene,  (HW * 0.5f + WALL_T * 0.5f), 0.3f, MID_Z, WALL_H, WALL_T, HOLE_LEN, off);
    // Start cap (z = +WALL_T/2)
    addWall(scene, 0.0f, 0.3f,  WALL_T * 0.5f,       WALL_H, HW + WALL_T * 2.0f, WALL_T, off);
    // End cap   (z = -HOLE_LEN - WALL_T/2)
    addWall(scene, 0.0f, 0.3f, -HOLE_LEN - WALL_T * 0.5f, WALL_H, HW + WALL_T * 2.0f, WALL_T, off);

    // ============================================================
    // GOLF CUP
    // Centred on the end flat at z=-15.5, y slightly above the
    // surface (top face of the 0.2-thick slab is at y=0.2; cup rim
    // is set at y=0.25 so its dark ring is visible against the green).
    // axis=1 → cylinder aligned with the Y axis.
    // ============================================================
    {
        glm::vec4 cupCentre = pt(0.0f, 0.25f, -15.5f, off);
        Cylinder<4>* cup = new Cylinder<4>(cupCentre, 0.18f, 0.35f, 24, 1);
        cup->setColour(CUP_R, CUP_G, CUP_B);
        scene.addShape(cup);
    }

    // ============================================================
    // FLAG POLE: thin cylinder rising from the cup rim upward.
    // ============================================================
    {
        glm::vec4 poleBase = pt(0.0f, 0.40f, -15.5f, off);
        glm::vec4 poleTip  = pt(0.0f, 1.70f, -15.5f, off);
        Cylinder<4>* pole = new Cylinder<4>(poleBase, poleTip, 0.025f, 8);
        pole->setColour(POLE_R, POLE_G, POLE_B);
        scene.addShape(pole);
    }

    // ============================================================
    // FLAG: small quad in the XY plane at the pole tip, extending
    // in the +X direction so it is visible when approaching along Z.
    // ============================================================
    {
        glm::vec4 fl_tl = pt(0.025f, 1.70f, -15.5f, off);
        glm::vec4 fl_tr = pt(0.55f,  1.70f, -15.5f, off);
        glm::vec4 fl_br = pt(0.55f,  1.45f, -15.5f, off);
        glm::vec4 fl_bl = pt(0.025f, 1.45f, -15.5f, off);
        Square<4>* flag = new Square<4>(fl_tl, fl_tr, fl_br, fl_bl);
        flag->setColour(FLAG_R, FLAG_G, FLAG_B);
        scene.addShape(flag);
    }
}
