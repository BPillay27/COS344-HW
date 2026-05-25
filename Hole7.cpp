// Hole7.cpp
// Terrain   : FLAT, RAMP_DOWN, FLAT, BUMP, FLAT
// Obstacles : Two wooden planks standing on the ramp, staggered left and right
//             to form a slalom the ball must weave between.
//
// Local Z: tee at z=0, cup at z=-17.5, far wall at z=-20.
// Local Y: upper surface 0.0 (Y_UPPER=-0.1), lower surface -1.0 (Y_LOWER=-1.1),
//          bump top surface -0.6 (Y_BUMP=-0.7).
//
// Terrain cross-section (z right-to-left, y up):
//
//  [upper flat]  [ramp]  [lower flat]  [bump]  [end flat + cup]
//  surf=0.0              surf=-1.0     -0.6     surf=-1.0
//  z=0..-4  \ramp z=-7   z=-7..-11   z=-11..-15   z=-15..-20

#include "Hole7.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Square.h"

static const int TURF_R = 9,   TURF_G = 139, TURF_B = 74;
static const int WALL_R = 101, WALL_G = 67,  WALL_B = 33;
static const int CUP_R  = 20,  CUP_G  = 20,  CUP_B  = 20;

static glm::vec4 pt(float x, float y, float z, glm::vec3 o) {
    return glm::vec4(x + o.x, y + o.y, z + o.z, 1.0f);
}

static void addFlatTurf(Figure& scene, float cx, float cy, float cz,
                        float width, float depth, glm::vec3 o) {
    Cube<4>* s = new Cube<4>(pt(cx, cy, cz, o), 0.2f, width, depth);
    s->setColour(TURF_R, TURF_G, TURF_B);
    scene.addShape(s);
}

static void addSlopeTurf(Figure& scene,
                         float fx, float fy, float fz,
                         float bx, float by, float bz,
                         float width, glm::vec3 o) {
    Square<4> front(pt(fx, fy, fz, o), 0.2f, width);
    Square<4> back (pt(bx, by, bz, o), 0.2f, width);
    Cube<4>* s = new Cube<4>(front, back);
    s->setColour(TURF_R, TURF_G, TURF_B);
    scene.addShape(s);
}

static void addWall(Figure& scene, float cx, float cy, float cz,
                    float h, float w, float d, glm::vec3 o) {
    Cube<4>* s = new Cube<4>(pt(cx, cy, cz, o), h, w, d);
    s->setColour(WALL_R, WALL_G, WALL_B);
    scene.addShape(s);
}

// ------------------------------------------------------------
void buildHole7(Figure& scene, glm::vec3 off, bool rot90) {

    const float HW     = 3.5f;
    const float WALL_T = 0.25f;
    const float WALL_H = 0.6f;
    const float LX     = HW * 0.5f + WALL_T * 0.5f;   // 1.875

    // Y slab centres (surface = cy + 0.1)
    const float Y_UPPER = -0.1f;   // surface at  0.0
    const float Y_LOWER = -1.1f;   // surface at -1.0
    const float Y_BUMP  = -0.7f;   // surface at -0.6  (bump top)

    // Z boundaries
    const float Z_RAMP_S = -4.0f;    // upper flat ends, ramp starts
    const float Z_RAMP_E = -7.0f;    // ramp ends, lower flat starts
    const float Z_MID_E  = -11.0f;   // lower flat ends, bump rise starts
    const float Z_RISE_E = -12.5f;   // bump rise ends, flat top starts
    const float Z_TOP_E  = -13.5f;   // bump flat top ends, fall starts
    const float Z_FALL_E = -15.0f;   // bump fall ends, end flat starts
    const float Z_CUP    = -17.5f;   // cup centre
    const float Z_FAR    = -20.0f;   // end cap

    // -------------------------------------------------------
    // TERRAIN
    // -------------------------------------------------------

    // 1. Upper flat (z = 0 to -4)
    addFlatTurf(scene, 0.0f, Y_UPPER, -2.0f, HW, 4.0f, off);

    // 2. RAMP_DOWN (z = -4 to -7): surface drops from 0.0 to -1.0
    addSlopeTurf(scene,
                 0.0f, Y_UPPER, Z_RAMP_S,
                 0.0f, Y_LOWER, Z_RAMP_E,
                 HW, off);

    // 3. Lower flat (z = -7 to -11)
    addFlatTurf(scene, 0.0f, Y_LOWER, -9.0f, HW, 4.0f, off);

    // 4. BUMP (z = -11 to -15): always three calls
    addSlopeTurf(scene,                       // rise (z = -11 to -12.5)
                 0.0f, Y_LOWER, Z_MID_E,
                 0.0f, Y_BUMP,  Z_RISE_E,
                 HW, off);
    addFlatTurf(scene, 0.0f, Y_BUMP, -13.0f, HW, 1.0f, off);   // flat top
    addSlopeTurf(scene,                       // fall (z = -13.5 to -15)
                 0.0f, Y_BUMP,  Z_TOP_E,
                 0.0f, Y_LOWER, Z_FALL_E,
                 HW, off);

    // 5. End flat (z = -15 to -20)
    addFlatTurf(scene, 0.0f, Y_LOWER, -17.5f, HW, 5.0f, off);

    // -------------------------------------------------------
    // BORDER WALLS
    // Wall cy formula: slab_cy + WALL_H * 0.5f - 0.1f
    //   (places wall bottom at slab bottom, top above surface)
    // -------------------------------------------------------

    // Tee cap (z = 0)
    {
        float cy = Y_UPPER + WALL_H * 0.5f - 0.1f;   //  0.1
        addWall(scene, 0.0f, cy, WALL_T * 0.5f,
                WALL_H, HW + WALL_T * 2.0f, WALL_T, off);
    }

    // Upper flat sides (z = 0 to -4)
    {
        float cy = Y_UPPER + WALL_H * 0.5f - 0.1f;   //  0.1
        addWall(scene, -LX, cy, -2.0f, WALL_H, WALL_T, 4.0f, off);
        addWall(scene,  LX, cy, -2.0f, WALL_H, WALL_T, 4.0f, off);
    }

    // Ramp sides (z = -4 to -7): approximate at average Y
    {
        float avgCY = (Y_UPPER + Y_LOWER) * 0.5f;      // -0.6
        float cy    = avgCY + WALL_H * 0.5f - 0.1f;    // -0.4
        addWall(scene, -LX, cy, -5.5f, WALL_H, WALL_T, 3.0f, off);
        addWall(scene,  LX, cy, -5.5f, WALL_H, WALL_T, 3.0f, off);
    }

    // Lower flat sides (z = -7 to -11)
    {
        float cy = Y_LOWER + WALL_H * 0.5f - 0.1f;    // -0.9
        addWall(scene, -LX, cy, -9.0f, WALL_H, WALL_T, 4.0f, off);
        addWall(scene,  LX, cy, -9.0f, WALL_H, WALL_T, 4.0f, off);
    }

    // Bump rise sides (z = -11 to -12.5): average Y
    {
        float avgCY = (Y_LOWER + Y_BUMP) * 0.5f;       // -0.9
        float cy    = avgCY + WALL_H * 0.5f - 0.1f;    // -0.7
        addWall(scene, -LX, cy, -11.75f, WALL_H, WALL_T, 1.5f, off);
        addWall(scene,  LX, cy, -11.75f, WALL_H, WALL_T, 1.5f, off);
    }

    // Bump flat top sides (z = -12.5 to -13.5)
    {
        float cy = Y_BUMP + WALL_H * 0.5f - 0.1f;     // -0.5
        addWall(scene, -LX, cy, -13.0f, WALL_H, WALL_T, 1.0f, off);
        addWall(scene,  LX, cy, -13.0f, WALL_H, WALL_T, 1.0f, off);
    }

    // Bump fall sides (z = -13.5 to -15): average Y
    {
        float avgCY = (Y_BUMP + Y_LOWER) * 0.5f;       // -0.9
        float cy    = avgCY + WALL_H * 0.5f - 0.1f;    // -0.7
        addWall(scene, -LX, cy, -14.25f, WALL_H, WALL_T, 1.5f, off);
        addWall(scene,  LX, cy, -14.25f, WALL_H, WALL_T, 1.5f, off);
    }

    // End flat sides (z = -15 to -20)
    {
        float cy = Y_LOWER + WALL_H * 0.5f - 0.1f;    // -0.9
        addWall(scene, -LX, cy, -17.5f, WALL_H, WALL_T, 5.0f, off);
        addWall(scene,  LX, cy, -17.5f, WALL_H, WALL_T, 5.0f, off);
    }

    // End cap (z = Z_FAR = -20)
    {
        float cy = Y_LOWER + WALL_H * 0.5f - 0.1f;    // -0.9
        addWall(scene, 0.0f, cy, Z_FAR - WALL_T * 0.5f,
                WALL_H, HW + WALL_T * 2.0f, WALL_T, off);
    }

    // -------------------------------------------------------
    // OBSTACLES: two wooden planks on the ramp, staggered for a slalom.
    // The ramp surface Y interpolates linearly from 0.0 at z=-4
    // to -1.0 at z=-7: surface_Y(z) = (z + 4) / 3.
    // Each plank stands upright: h=0.4 (tall), w=0.15 (thin in X), d=1.5 (along Z).
    // Plank bottom rests on the slope surface.
    // -------------------------------------------------------
    {
        const float plankH = 0.4f, plankW = 0.15f, plankD = 1.5f;

        // Plank 1: left side of ramp, centred at z = -5.0
        // surface_Y(-5) = (-5 + 4) / 3 = -0.333
        const float p1SY = (-5.0f + 4.0f) / 3.0f;    // -0.333
        float p1CY = p1SY + plankH * 0.5f;            // -0.133
        Cube<4>* plank1 = new Cube<4>(pt(-0.8f, p1CY, -5.0f, off),
                                      plankH, plankW, plankD);
        plank1->setColour(WALL_R, WALL_G, WALL_B);
        scene.addShape(plank1);

        // Plank 2: right side of ramp, centred at z = -6.0
        // surface_Y(-6) = (-6 + 4) / 3 = -0.667
        const float p2SY = (-6.0f + 4.0f) / 3.0f;    // -0.667
        float p2CY = p2SY + plankH * 0.5f;            // -0.467
        Cube<4>* plank2 = new Cube<4>(pt( 0.8f, p2CY, -6.0f, off),
                                      plankH, plankW, plankD);
        plank2->setColour(WALL_R, WALL_G, WALL_B);
        scene.addShape(plank2);
    }

    // -------------------------------------------------------
    // CUP (always last)
    // -------------------------------------------------------
    {
        float surfaceY = Y_LOWER + 0.1f;               // -1.0
        glm::vec4 cen  = pt(0.0f, surfaceY + 0.075f, Z_CUP, off);
        Cylinder<4>* cup = new Cylinder<4>(cen, 0.18f, 0.15f, 24, 1);
        cup->setColour(CUP_R, CUP_G, CUP_B);
        scene.addShape(cup);
    }
}
