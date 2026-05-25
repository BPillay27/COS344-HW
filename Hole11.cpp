// Hole11.cpp
// Constructs all geometry for Hole 11 of the 19th Hole Putt-Putt course.
//
// Hole 11 description (from the final report):
//   Terrain  : FLAT, CLIFF_DOWN, FLAT, RAMP_DOWN, FLAT
//              - Start flat at the upper level.
//              - Vertical cliff drop (1.2 units) to the middle level.
//              - Middle flat: contains the cup and both V-concrete obstacles.
//              - Ramp falls away past the cup to the lower pitfall flat.
//              - Pitfall flat: overhit ball rolls here and must be played back up.
//   Obstacles: two V-shaped concrete bumpers flanking the cup, one per side.
//
// Local coordinate system (worldOffset shifts into world space):
//   X = width  (hole is 3.5 units wide, centred on x=0)
//   Y = height (up)
//   Z = length (tee at z=0, cup at z=-7.5, far pitfall wall at z=-15.0)
//
// Terrain cross-section (z left->right, y up):
//
//  [upper flat]        [middle flat + cup + V-obstacles]  [pitfall]
//  y=0.1               y=-0.25                             y=-0.45
//  z=0..-3  |cliff|    z=-3..-9   |ramp| z=-9..-11         z=-11..-15
//            face      (cliff 0.35 drop)  (ramp 0.2 drop)

#include "Hole11.h"
#include "Hole11Helpers.h"
#include "Shapes/Cylinder.h"

static const int H11_CUP_R = 20, H11_CUP_G = 20, H11_CUP_B = 20;

void buildHole11(Figure& scene, glm::vec3 off, bool rot180) {

    // -----------------------------------------------------------------------
    // Key dimensions
    // -----------------------------------------------------------------------
    const float HW          = 3.5f;
    const float WALL_T      = 0.25f;
    const float WALL_H      = 0.2f;  // reduced: was 0.3
    const float LX          = HW * 0.5f + WALL_T * 0.5f;  // 1.875

    // Y surface centres (slab is 0.2 thick, so surface = cy + 0.1)
    //   cliff drop: Y_UPPER - Y_MID = 0.1 - (-0.25) = 0.35 (was 0.6)
    //   ramp  drop: Y_MID - Y_LOWER = -0.25 - (-0.45) = 0.2 (was 0.5)
    const float Y_UPPER = 0.1f;    // surface at  0.2  (unchanged)
    const float Y_MID   = -0.25f;  // surface at -0.15 (was -0.5)
    const float Y_LOWER = -0.45f;  // surface at -0.35 (was -1.0)

    // Z boundaries
    const float Z_CLIFF    = -3.0f;
    const float Z_MID_END  = -9.0f;
    const float Z_RAMP_END = -11.0f;
    const float Z_FAR      = -15.0f;
    const float Z_CUP      = -7.5f;

    // -----------------------------------------------------------------------
    // TERRAIN
    // -----------------------------------------------------------------------

    // Section 1: upper FLAT (z = 0 to -3)
    h11FlatTurf(scene, 0.0f, Y_UPPER, -1.5f, HW, 3.0f, off, rot180);

    // Cliff retaining face at z = Z_CLIFF: fills the 0.6-unit drop visually
    {
        float dropH = Y_UPPER - Y_MID;                  // 1.2
        float midCY = Y_MID + dropH * 0.5f;             // -0.5
        h11Concrete(scene, 0.0f, midCY, Z_CLIFF - WALL_T * 0.5f,
                    dropH, HW, WALL_T, off, rot180);
    }

    // Section 2: middle FLAT (z = -3 to -9)
    h11FlatTurf(scene, 0.0f, Y_MID, -6.0f, HW, 6.0f, off, rot180);

    // Section 3: RAMP_DOWN (z = -9 to -11)
    h11SlopeTurf(scene,
                 0.0f, Y_MID,   Z_MID_END,
                 0.0f, Y_LOWER, Z_RAMP_END,
                 HW, off, rot180);

    // Section 4: pitfall FLAT (z = -11 to -15)
    h11FlatTurf(scene, 0.0f, Y_LOWER, -13.0f, HW, 4.0f, off, rot180);

    // -----------------------------------------------------------------------
    // BORDER WALLS
    // Each terrain segment has its own side walls so the heights track the
    // terrain. All walls use the wooden colour defined in Hole11Helpers.cpp.
    // -----------------------------------------------------------------------

    // Tee cap (z = 0)
    {
        float cy = Y_UPPER + WALL_H * 0.5f - 0.1f;
        h11Wall(scene, 0.0f, cy, WALL_T * 0.5f,
                WALL_H, HW + WALL_T * 2.0f, WALL_T, off, rot180);
    }

    // Upper flat sides (z = 0 to Z_CLIFF)
    {
        float cy = Y_UPPER + WALL_H * 0.5f - 0.1f;
        h11Wall(scene, -LX, cy, -1.5f, WALL_H, WALL_T, 3.0f, off, rot180);
        h11Wall(scene,  LX, cy, -1.5f, WALL_H, WALL_T, 3.0f, off, rot180);
    }

    // Cliff-drop side patches: cover the 0.6-unit vertical gap in the wall
    {
        float dropH = Y_UPPER - Y_MID;
        float midCY = Y_MID + dropH * 0.5f;
        h11Wall(scene, -LX, midCY, Z_CLIFF - WALL_T * 0.5f,
                dropH, WALL_T, WALL_T, off, rot180);
        h11Wall(scene,  LX, midCY, Z_CLIFF - WALL_T * 0.5f,
                dropH, WALL_T, WALL_T, off, rot180);
    }

    // Middle flat sides (z = Z_CLIFF to Z_MID_END)
    {
        float cy = Y_MID + WALL_H * 0.5f - 0.1f;
        h11Wall(scene, -LX, cy, -6.0f, WALL_H, WALL_T, 6.0f, off, rot180);
        h11Wall(scene,  LX, cy, -6.0f, WALL_H, WALL_T, 6.0f, off, rot180);
    }

    // Ramp sides (z = Z_MID_END to Z_RAMP_END)
    // Approximated at the average Y of the two ends so no gap opens.
    {
        float avgCY = (Y_MID + Y_LOWER) * 0.5f + WALL_H * 0.5f - 0.1f;
        h11Wall(scene, -LX, avgCY, -10.0f, WALL_H, WALL_T, 2.0f, off, rot180);
        h11Wall(scene,  LX, avgCY, -10.0f, WALL_H, WALL_T, 2.0f, off, rot180);
    }

    // Pitfall sides (z = Z_RAMP_END to Z_FAR)
    {
        float cy = Y_LOWER + WALL_H * 0.5f - 0.1f;
        h11Wall(scene, -LX, cy, -13.0f, WALL_H, WALL_T, 4.0f, off, rot180);
        h11Wall(scene,  LX, cy, -13.0f, WALL_H, WALL_T, 4.0f, off, rot180);
    }

    // Far cap (z = Z_FAR)
    {
        float cy = Y_LOWER + WALL_H * 0.5f - 0.1f;
        h11Wall(scene, 0.0f, cy, Z_FAR - WALL_T * 0.5f,
                WALL_H, HW + WALL_T * 2.0f, WALL_T, off, rot180);
    }

    // -----------------------------------------------------------------------
    // V-CONCRETE OBSTACLES
    // Each V is built from two angled arms sharing an inner tip near the cup.
    // The tip is offset 0.55 units from centre in X and 0.3 units forward in Z.
    // Each arm's outer base is 1.25 units from centre in X.
    //
    // Visual layout (top-down, Z increases upward, X increases rightward):
    //
    //   wall    base      tip   cup   tip      base    wall
    //   ----  \         /   [CUP]   \         /  ----
    //          \  left /             \ right /
    //           V arm                 V arm
    // -----------------------------------------------------------------------
    const float V_SURF = Y_MID + 0.1f;   // middle flat surface = Y_MID + 0.1 = -0.15

    // Left V — tip at outer wall, arms spread inward toward cup
    h11VArm(scene, -1.25f, Z_CUP - 0.3f, -0.55f, Z_CUP - 1.2f, V_SURF, off, rot180);
    h11VArm(scene, -1.25f, Z_CUP - 0.3f, -0.55f, Z_CUP + 1.2f, V_SURF, off, rot180);

    // Right V — tip at outer wall, arms spread inward toward cup
    h11VArm(scene,  1.25f, Z_CUP - 0.3f,  0.55f, Z_CUP - 1.2f, V_SURF, off, rot180);
    h11VArm(scene,  1.25f, Z_CUP - 0.3f,  0.55f, Z_CUP + 1.2f, V_SURF, off, rot180);

    // -----------------------------------------------------------------------
    // GOLF CUP
    // Sits on the middle flat, centred at Z_CUP.
    // Radius 0.18, height 0.15, just proud of the turf.
    // -----------------------------------------------------------------------
    {
        float cupCY = V_SURF + 0.075f;   // slab centre = surface + half-height
        glm::vec4 centre = h11pt(0.0f, cupCY, Z_CUP, off, rot180);
        Cylinder<4>* cup = new Cylinder<4>(centre, 0.18f, 0.15f, 24, 1);
        cup->setColour(H11_CUP_R, H11_CUP_G, H11_CUP_B);
        scene.addShape(cup);
    }
}

/***
Only takie in the height adjustments
*/