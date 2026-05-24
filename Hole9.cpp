// Hole9.cpp
// L-shaped hole: FLAT, RAMP_UP, FLAT  (90 degrees clockwise from original design)
//
// Top-down layout (X right, Z into screen):
//
//   x=0          x=4     x=7        x=10.5
// z=-1.75  +------+-------+-----------+
//          | Leg1 | RAMP  |  Corner   |
// z=+1.75  +------+-------+---+       |
//                            | Leg 2  |
//                            |  (+Z)  |
// z=+8.75                   +---------+  (cup here)
//
// Tee at x=0, ball travels +X along Leg1, up the ramp to the corner,
// then the L bends into Leg2 going +Z, cup at far end.

#include "Hole9.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Square.h"

// Colours (0-255)
static const int TURF9_R = 34,  TURF9_G = 139, TURF9_B = 34;
static const int WALL9_R = 101, WALL9_G = 67,  WALL9_B = 33;
static const int CUP9_R  = 20,  CUP9_G  = 20,  CUP9_B  = 20;
static const int POLE9_R = 200, POLE9_G = 200, POLE9_B = 200;
static const int FLAG9_R = 220, FLAG9_G = 30,  FLAG9_B = 30;

static glm::vec4 pt9(float x, float y, float z, glm::vec3 off) {
    return glm::vec4(x + off.x, y + off.y, z + off.z, 1.0f);
}

static void addFlat9(Figure& scene,
                     float cx, float cy, float cz,
                     float w, float d, glm::vec3 off) {
    Cube<4>* s = new Cube<4>(pt9(cx, cy, cz, off), 0.2f, w, d);
    s->setColour(TURF9_R, TURF9_G, TURF9_B);
    scene.addShape(s);
}

static void addWall9(Figure& scene,
                     float cx, float cy, float cz,
                     float h, float w, float d, glm::vec3 off) {
    Cube<4>* s = new Cube<4>(pt9(cx, cy, cz, off), h, w, d);
    s->setColour(WALL9_R, WALL9_G, WALL9_B);
    scene.addShape(s);
}

void buildHole9(Figure& scene, glm::vec3 off) {
    const float HW     = 3.5f;   // corridor width
    const float WALL_T = 0.25f;
    const float WALL_H = 0.6f;
    const float LW     = HW * 0.5f + WALL_T * 0.5f;  // 1.875 — wall-centre offset

    // ============================================================
    // TERRAIN
    // ============================================================

    // Leg 1 FLAT  (x = 0 to 4, z = -1.75 to 1.75)
    addFlat9(scene, 2.0f, 0.1f, 0.0f, 4.0f, HW, off);

    // RAMP_UP  (x = 4 to 7)
    // Cross-sections are in the ZY plane; front face at x=4 (ground),
    // back face at x=7 (elevated to y-centre 0.7).
    {
        Square<4> frontFace(
            pt9(4.0f, 0.2f, -1.75f, off),   // tl
            pt9(4.0f, 0.2f,  1.75f, off),   // tr
            pt9(4.0f, 0.0f,  1.75f, off),   // br
            pt9(4.0f, 0.0f, -1.75f, off)    // bl
        );
        Square<4> backFace(
            pt9(7.0f, 0.8f, -1.75f, off),
            pt9(7.0f, 0.8f,  1.75f, off),
            pt9(7.0f, 0.6f,  1.75f, off),
            pt9(7.0f, 0.6f, -1.75f, off)
        );
        Cube<4>* ramp = new Cube<4>(frontFace, backFace);
        ramp->setColour(TURF9_R, TURF9_G, TURF9_B);
        scene.addShape(ramp);
    }

    // Corner FLAT  (x = 7 to 10.5, z = -1.75 to 1.75) — top of the ramp
    addFlat9(scene, 8.75f, 0.7f, 0.0f, 3.5f, HW, off);

    // Leg 2 FLAT  (x = 7 to 10.5, z = 1.75 to 8.75) — the +Z arm of the L
    addFlat9(scene, 8.75f, 0.7f, 5.25f, 3.5f, 7.0f, off);

    // ============================================================
    // BORDER WALLS
    // ============================================================

    // --- Start cap (tee end, X-aligned at x = -0.125) ---
    // Z-aligned slab: thin in X, wide in Z
    addWall9(scene, -WALL_T * 0.5f, 0.3f, 0.0f,
             WALL_H, WALL_T, HW + WALL_T * 2.0f, off);

    // --- South outer wall  (z = -LW, runs +X) ---
    // Leg 1 portion (x = 0..4): base height
    addWall9(scene,  2.0f, 0.3f,  -LW, WALL_H, 4.0f, WALL_T, off);
    // Ramp portion  (x = 4..7): extra height for rising terrain
    addWall9(scene,  5.5f, 0.3f,  -LW, WALL_H, 3.0f, WALL_T, off);
    addWall9(scene,  5.5f, 0.6f,  -LW, WALL_H, 3.0f, WALL_T, off);  // extra raised
    // Corner portion (x = 7..10.5): elevated
    addWall9(scene,  8.75f, 0.9f, -LW, WALL_H, 3.5f, WALL_T, off);

    // --- North inner wall of Leg 1 + Ramp  (z = +LW, runs +X, stops at x=7) ---
    addWall9(scene,  2.0f, 0.3f,  LW, WALL_H, 4.0f, WALL_T, off);
    addWall9(scene,  5.5f, 0.3f,  LW, WALL_H, 3.0f, WALL_T, off);
    addWall9(scene,  5.5f, 0.6f,  LW, WALL_H, 3.0f, WALL_T, off);   // extra raised

    // --- Inner west wall of Leg 2  (x = 6.875, runs +Z from z=1.75 to 8.75) ---
    // Z-aligned slab at the inner corner of the L
    addWall9(scene, 7.0f - WALL_T * 0.5f, 0.9f, 5.25f,
             WALL_H, WALL_T, 7.0f + WALL_T * 2.0f, off);

    // --- East outer wall  (x = 10.625, runs full z-span) ---
    // Covers z = -LW to 8.75 (leg1+ramp+corner southern portion)
    addWall9(scene, 10.5f + WALL_T * 0.5f, 0.3f, 3.5f,
             WALL_H, WALL_T, 10.75f, off);
    // Elevated section for the corner and Leg 2 (z = -LW to 8.875)
    addWall9(scene, 10.5f + WALL_T * 0.5f, 0.9f, 3.5f,
             WALL_H, WALL_T, 10.75f, off);

    // --- North end cap (cup end, z = 8.875) ---
    // X-aligned slab covering x = 7 to 10.5
    addWall9(scene, 8.75f, 0.9f, 8.75f + WALL_T * 0.5f,
             WALL_H, 3.5f + WALL_T * 2.0f, WALL_T, off);

    // ============================================================
    // GOLF CUP  (on Leg 2 flat, cup centred at x=9.0, z=7.5)
    // ============================================================
    {
        glm::vec4 cupCentre = pt9(9.0f, 0.85f, 7.5f, off);
        Cylinder<4>* cup = new Cylinder<4>(cupCentre, 0.18f, 0.05f, 24, 1);
        cup->setColour(CUP9_R, CUP9_G, CUP9_B);
        scene.addShape(cup);
    }
}
