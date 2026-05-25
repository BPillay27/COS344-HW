# Hole Building Framework

This file is the mandatory reference for any AI session building a hole file for the 19th Hole Putt-Putt project. Read every section before writing a single line of code. Do not infer, guess, or invent anything not written here.

---

## 1. Non-negotiable rules

- Every hole is two files: `HoleN.h` and `HoleN.cpp`. No other structure.
- `HoleN.h` must be under 15 lines.
- `HoleN.cpp` must be under 300 lines. If it will exceed 250 lines, split helpers into `HoleNHelpers.h` and `HoleNHelpers.cpp` before writing `HoleN.cpp`. See Section 9 for the split pattern.
- Only these includes are allowed: `Shapes/Cube.h`, `Shapes/Cylinder.h`, `Shapes/Sphere.h`, `Shapes/Square.h`, `Shapes/Circle.h`, `Shapes/Triangle.h`,`Shapes/TriangularPrism.h`, `Shapes/SquarePyramid.h`, `glm/glm.hpp`, `Figure.h`.
- No `std::`, no `iostream`, no external libraries.
- All colour values use the exact RGB integers from Section 4. Never invent colours.
- The only template parameter used is `<4>`. Always `Cube<4>`, `Cylinder<4>`, `Sphere<4>`, etc.
- All heap objects are `new`-allocated and passed to `scene.addShape()`. Never stack-allocate a shape.
- Use commas, never em-dashes, in any comment or prose.

---

## 2. Coordinate system

```
X = width   (hole is 3.5 units wide, centred on x=0)
Y = height  (up is positive)
Z = length  (tee at z=0, cup at some negative z)
```

The `worldOffset` (called `off` inside the function) shifts the entire hole into world space. Every point is built with the `pt()` helper, which bakes `off` in.

---

## 3. The five helper functions (copy verbatim, change nothing)

These are the same in every hole file. Copy them exactly. Do not rename, do not change parameter order, do not change the slab thickness.

```cpp
// Build a vec4 with world offset baked in.
static glm::vec4 pt(float x, float y, float z, glm::vec3 o) {
    return glm::vec4(x + o.x, y + o.y, z + o.z, 1.0f);
}

// Axis-aligned turf slab, 0.2 units thick in Y.
// cx, cy, cz = centre of the slab.
// cy = (intended surface Y) - 0.1
static void addFlatTurf(Figure& scene,
                        float cx, float cy, float cz,
                        float width, float depth,
                        glm::vec3 o) {
    Cube<4>* slab = new Cube<4>(pt(cx, cy, cz, o), 0.2f, width, depth);
    slab->setColour(TURF_R, TURF_G, TURF_B);
    scene.addShape(slab);
}

// Angled turf slab connecting two face centres at different Y heights.
// (fx,fy,fz) = front face centre, nearer to tee, higher Z value.
// (bx,by,bz) = back  face centre, further from tee, lower Z value.
static void addSlopeTurf(Figure& scene,
                         float fx, float fy, float fz,
                         float bx, float by, float bz,
                         float width,
                         glm::vec3 o) {
    Square<4> front(pt(fx, fy, fz, o), 0.2f, width);
    Square<4> back (pt(bx, by, bz, o), 0.2f, width);
    Cube<4>* ramp = new Cube<4>(front, back);
    ramp->setColour(TURF_R, TURF_G, TURF_B);
    scene.addShape(ramp);
}

// Border wall slab (wooden material).
static void addWall(Figure& scene,
                    float cx, float cy, float cz,
                    float h, float w, float d,
                    glm::vec3 o) {
    Cube<4>* wall = new Cube<4>(pt(cx, cy, cz, o), h, w, d);
    wall->setColour(WALL_R, WALL_G, WALL_B);
    scene.addShape(wall);
}

// Concrete slab (cliff faces, V-obstacles, any non-wooden structure).
static void addConcrete(Figure& scene,
                        float cx, float cy, float cz,
                        float h, float w, float d,
                        glm::vec3 o) {
    Cube<4>* block = new Cube<4>(pt(cx, cy, cz, o), h, w, d);
    block->setColour(CONCRETE_R, CONCRETE_G, CONCRETE_B);
    scene.addShape(block);
}
```

---

## 4. Colour constants (use these exact values, always)

Declare these as `static const int` at file scope, before any function.

```cpp
static const int TURF_R     =   9, TURF_G     = 139, TURF_B     =  74;
static const int WALL_R     = 101, WALL_G     =  67, WALL_B     =  33;
static const int CONCRETE_R = 179, CONCRETE_G = 177, CONCRETE_B = 176;
static const int CUP_R      =  20, CUP_G      =  20, CUP_B      =  20;
```

Reference: turf is dark green, wall is dark wood brown, concrete is grey, cup is near-black.

---

## 5. The Y rule (most common source of errors)

Every slab is 0.2 units thick. The `cy` argument is the slab centre, not the surface.

```
surface Y  =  cy + 0.1
cy         =  (intended surface Y) - 0.1
```

Examples:

| Intended surface | Pass as cy |
|---|---|
| 0.0 (ground) | -0.1 |
| 0.2 | 0.1 |
| 1.0 (top of bump) | 0.9 |
| -1.0 (lower level) | -1.1 |
| -2.0 (pitfall) | -2.1 |

For a slope, both the front cy and back cy follow this rule independently.

---

## 6. Segment types

Every hole is a chain of these segments along Z (tee at z=0, cup at negative z). Chain them in order from tee to cup.

### FLAT

```cpp
// A flat platform from zA to zB.
// centre Z = (zA + zB) / 2
// depth    = abs(zB - zA)
addFlatTurf(scene, 0.0f, cy, centrZ, HW, depth, off);
```

### RAMP_UP and RAMP_DOWN

Built with `addSlopeTurf`. The front face is nearer the tee (higher Z), the back face is further from the tee (lower Z).

```cpp
// RAMP_UP: terrain rises from zA (Y=startCY) to zB (Y=endCY), endCY > startCY
addSlopeTurf(scene,
             0.0f, startCY, zA,    // front face: at zA, lower Y
             0.0f, endCY,   zB,    // back  face: at zB, higher Y
             HW, off);

// RAMP_DOWN: terrain falls from zA (Y=startCY) to zB (Y=endCY), endCY < startCY
addSlopeTurf(scene,
             0.0f, startCY, zA,    // front face: at zA, higher Y
             0.0f, endCY,   zB,    // back  face: at zB, lower Y
             HW, off);
```

### BUMP

A bump is always three calls: rise, flat top, fall. Never one call.

```cpp
// Rise from groundCY to topCY over [z0, z1]
addSlopeTurf(scene, 0.0f, groundCY, z0,  0.0f, topCY, z1,  HW, off);
// Flat top from z1 to z2
addFlatTurf (scene, 0.0f, topCY, (z1+z2)*0.5f, HW, z2-z1, off);
// Fall from topCY to groundCY over [z2, z3]
addSlopeTurf(scene, 0.0f, topCY, z2,  0.0f, groundCY, z3,  HW, off);
```

### CLIFF_DOWN

A vertical retaining face at a single Z value. Use `addConcrete`, not `addSlopeTurf`.

```cpp
// Fills the vertical gap between upperCY and lowerCY at zCliff.
float dropH  = (upperCY + 0.1f) - (lowerCY + 0.1f); // surface diff
float faceCY = lowerCY + 0.1f + dropH * 0.5f;        // midpoint
addConcrete(scene, 0.0f, faceCY, zCliff - WALL_T * 0.5f,
            dropH, HW, WALL_T, off);
```

### WATER

A flat plane at a lower Y with a different colour. Use a Cube with blue material.

```cpp
Cube<4>* water = new Cube<4>(pt(0.0f, waterCY, centrZ, off), 0.1f, HW, depth);
water->setColour(128, 195, 240);
scene.addShape(water);
```

### BRIDGE_OVER_WATER

Two thin horizontal slabs with a gap between them, placed above a WATER segment.

```cpp
// Left plank
addFlatTurf(scene, -HW*0.25f, bridgeCY, centrZ, HW*0.45f, depth, off);
// Right plank
addFlatTurf(scene,  HW*0.25f, bridgeCY, centrZ, HW*0.45f, depth, off);
```

---

## 7. Border wall rules

Walls are the most error-prone part. Follow these rules exactly.

### Constants to declare at the top of every build function

```cpp
const float HW     = 3.5f;
const float WALL_T = 0.25f;   // wall thickness
const float WALL_H = 0.6f;    // base wall height
const float LX     = HW * 0.5f + WALL_T * 0.5f;  // = 1.875, X centre of side walls
```

### Tee cap (z = 0)

```cpp
float capCY = groundCY + WALL_H * 0.5f;
addWall(scene, 0.0f, capCY, WALL_T * 0.5f,
        WALL_H, HW + WALL_T * 2.0f, WALL_T, off);
```

### End cap (z = zFar)

```cpp
float capCY = finalCY + WALL_H * 0.5f;
addWall(scene, 0.0f, capCY, zFar - WALL_T * 0.5f,
        WALL_H, HW + WALL_T * 2.0f, WALL_T, off);
```

### Side walls for a flat segment

```cpp
float wallCY = segSurfaceCY + WALL_H * 0.5f;  // segSurfaceCY = cy + 0.1 - 0.1 = cy
addWall(scene, -LX, wallCY, segCentreZ, WALL_H, WALL_T, segDepth, off);
addWall(scene,  LX, wallCY, segCentreZ, WALL_H, WALL_T, segDepth, off);
```

### Extra wall height above a bump or raised terrain

When terrain rises above `WALL_H` above the base level, add extra wall sections. For a bump top at `topCY`:

```cpp
// Extra covers y = (baseSurface + WALL_H) to (bumpSurface + WALL_H)
float extraH  = (topCY + 0.1f) - (groundCY + 0.1f);   // height of extra section
float extraCY = groundCY + 0.1f + WALL_H + extraH * 0.5f;
addWall(scene, -LX, extraCY, segCentreZ, extraH, WALL_T, segDepth, off);
addWall(scene,  LX, extraCY, segCentreZ, extraH, WALL_T, segDepth, off);
```

### Side wall patches at a cliff transition

When Y drops instantly at a cliff, add narrow wall patches to fill the vertical gap on each side.

```cpp
float dropH  = upperCY - lowerCY;
float patchCY = lowerCY + dropH * 0.5f;
addWall(scene, -LX, patchCY, zCliff - WALL_T * 0.5f,
        dropH, WALL_T, WALL_T, off);
addWall(scene,  LX, patchCY, zCliff - WALL_T * 0.5f,
        dropH, WALL_T, WALL_T, off);
```

---

## 8. The cup (always last, always identical)

```cpp
// Place this as the final block in every build function.
float surfaceY = finalSegCY + 0.1f;   // top of the turf at cup location
glm::vec4 cupCentre = pt(0.0f, surfaceY + 0.075f, Z_CUP, off);
Cylinder<4>* cup = new Cylinder<4>(cupCentre, 0.18f, 0.15f, 24, 1);
cup->setColour(CUP_R, CUP_G, CUP_B);
scene.addShape(cup);
```

Radius is always 0.18. Height is always 0.15. Slices are always 24. Stacks are always 1. Centre Y is always `surfaceY + 0.075` (half of 0.15). Never deviate.

---

## 9. File split rule

If `HoleN.cpp` will exceed 250 lines, extract all static helpers into a sibling pair before writing the main file.

`HoleNHelpers.h` declares: `pt`, `addFlatTurf`, `addSlopeTurf`, `addWall`, `addConcrete`, plus any hole-specific helpers (e.g. `addVArm` for Hole 11).

`HoleNHelpers.cpp` defines them and owns the colour constants.

`HoleN.cpp` includes `HoleNHelpers.h` instead of redefining them, and only contains the `buildHoleN` function body.

---

## 10. Procedural obstacle reference

Use these for shapes described as primitives in the report.

| Obstacle | How to build |
|---|---|
| Triangular deflector | `Cube<4>` via two-Square constructor, one narrow face and one wider face at an angle |
| Wooden plank | `Cube<4>(pt(...), 0.4f, 0.15f, 1.5f)` with WALL colour |
| Rock | `Sphere<4>(pt(...), 0.25f, 8, 16)` with `setColour(120,110,100)` |
| Pumpkin | `Sphere<4>` radius 0.28 with `setColour(200,100,20)`, plus thin Cylinder stem |
| Log (on its side) | `Cylinder<4>` with axis along X, placed at `surfaceY + radius` |
| Pipe (vertical) | `Cylinder<4>` standing upright, small radius |
| V-concrete arm | `Cube<4>` via two-Square constructor, CONCRETE colour, see Hole 11 for the exact pattern |
| Overhead railing | Thin `Cube<4>` high in Y, WALL_T thickness, spanning the hole width |

---

## 11. Complete hole-by-hole plan

The segment chain and key obstacles for all 18 holes, derived from the final report.

```
Hole  Segment chain                                    Key obstacles
1     FLAT, BUMP, FLAT                                 Dog statue on bump
2     FLAT, BUMP, FLAT, RAMP_UP, FLAT                  Frog statue mid
3     FLAT                                             Dog statue, rock near cup
4     FLAT, TWO_LEVEL_DROP, FLAT                       Tri prisms, pipe cylinders
5     FLAT, RAMP_UP, FLAT, RAMP_DOWN, FLAT             Windmill (Prac 3 .obj import)
6     FLAT, BUMP, FLAT                                 No obstacle, terrain only
7     FLAT, RAMP_DOWN, FLAT, BUMP, FLAT                Two wooden planks on slope
8     FLAT, BUMP, FLAT, BUMP, FLAT         (done)      Brick border walls
9     FLAT, RAMP_UP, FLAT                              Tiny human statue
10    FLAT                                             Two tri deflectors, dog statue
11    FLAT, CLIFF_DOWN, FLAT, RAMP_DOWN, FLAT (done)   Two V-concrete flanking cup
12    FLAT, RAMP_UP, FLAT                              Tri prism corner, damaged bus .obj
13    FLAT, WATER, FLAT                                Pumpkin, dove spheres, corner tris
14    FLAT, RAMP_UP, FLAT, RAMP_DOWN, FLAT             Pumpkin sphere
15    FLAT, BRIDGE_OVER_WATER, FLAT                    Frog .obj, tri prisms at turns
16    FLAT, RAMP_DOWN, FLAT, RAMP_UP, FLAT             Log cylinder, yellow slab cube, pot
17    FLAT, RAMP_UP, FLAT                              Deer .obj statues
18    FLAT, RAMP_UP, CLIFF_DOWN, TUNNEL, FLAT          Overhead railing cube
```

Holes 8 and 11 are already built. Use them as reference before starting any other hole.

---

## 12. File template (paste and fill in)

Use this exact template. Replace every `N` with the hole number. Fill in the coordinate sections. Do not add anything not in this template unless it is an obstacle-specific helper.

### HoleN.h

```cpp
#pragma once
// HoleN.h
// Hole N of the 19th Hole Putt-Putt course.
#include <glm/glm.hpp>
#include "Figure.h"

void buildHoleN(Figure& scene, glm::vec3 worldOffset);
```

### HoleN.cpp

```cpp
// HoleN.cpp
// Terrain   : [list your segment chain here]
// Obstacles : [list your obstacles here]
//
// Local Z: tee at z=0, cup at z=Z_CUP, far wall at z=Z_FAR.
// Local Y: ground surface at Y_GROUND, [other levels if any].

#include "HoleN.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Square.h"

static const int TURF_R=9,   TURF_G=139, TURF_B=74;
static const int WALL_R=101, WALL_G=67,  WALL_B=33;
static const int CONCRETE_R=179, CONCRETE_G=177, CONCRETE_B=176;
static const int CUP_R=20,  CUP_G=20,   CUP_B=20;

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
void buildHoleN(Figure& scene, glm::vec3 off) {

    const float HW     = 3.5f;
    const float WALL_T = 0.25f;
    const float WALL_H = 0.6f;
    const float LX     = HW * 0.5f + WALL_T * 0.5f;

    // Terrain Y centres (surface = cy + 0.1)
    const float Y_GROUND = 0.1f;   // adjust per hole
    // const float Y_UPPER  = ...  // add more levels if needed

    // Z boundaries (fill these in from your paper sketch)
    const float Z_CUP = -0.0f;   // FILL IN
    const float Z_FAR = -0.0f;   // FILL IN

    // -------------------------------------------------------
    // TERRAIN
    // -------------------------------------------------------
    // Write segments here in Z order from tee to cup.
    // addFlatTurf  for flat sections
    // addSlopeTurf for ramps
    // See framework Section 6 for BUMP and CLIFF patterns.

    // -------------------------------------------------------
    // BORDER WALLS
    // -------------------------------------------------------
    // Tee cap
    addWall(scene, 0.0f, Y_GROUND + WALL_H * 0.5f, WALL_T * 0.5f,
            WALL_H, HW + WALL_T * 2.0f, WALL_T, off);
    // End cap
    addWall(scene, 0.0f, Y_GROUND + WALL_H * 0.5f, Z_FAR - WALL_T * 0.5f,
            WALL_H, HW + WALL_T * 2.0f, WALL_T, off);
    // Side walls: one pair per terrain segment

    // -------------------------------------------------------
    // OBSTACLES
    // -------------------------------------------------------
    // One block per obstacle, procedural or .obj loader call.

    // -------------------------------------------------------
    // CUP (always last)
    // -------------------------------------------------------
    {
        float surfaceY  = Y_GROUND + 0.1f;
        glm::vec4 cen   = pt(0.0f, surfaceY + 0.075f, Z_CUP, off);
        Cylinder<4>* cup = new Cylinder<4>(cen, 0.18f, 0.15f, 24, 1);
        cup->setColour(CUP_R, CUP_G, CUP_B);
        scene.addShape(cup);
    }
}
```

---

## 13. Pre-flight checklist (run before marking any hole as done)

- [ ] `HoleN.h` is under 15 lines and declares exactly `buildHoleN`.
- [ ] `HoleN.cpp` is under 300 lines. If it was split, all three files compile together.
- [ ] Turf colour is exactly 9, 139, 74. Wall colour is 101, 67, 33. Cup is 20, 20, 20.
- [ ] Every `cy` argument = (intended surface Y) - 0.1.
- [ ] Every segment's Z boundaries were written on paper before coding and match the call arguments.
- [ ] A tee cap wall and an end cap wall exist.
- [ ] Side walls cover every terrain segment for the full Z length of that segment.
- [ ] Where terrain rises above the base level (bumps, upper levels), extra wall height is added.
- [ ] Where terrain drops at a cliff, narrow wall patches fill the vertical gap on both sides.
- [ ] The cup is a `Cylinder<4>`, radius 0.18, height 0.15, 24 slices, 1 stack, centre Y = surfaceY + 0.075.
- [ ] `buildHoleN` is called in `main.cpp` with the correct world offset from `course.txt`.
- [ ] The hole builds and runs without NaN or Inf warnings in stdout (Shape.cpp prints these on corrupt transforms).

---

## 14. What the AI must not do

- Do not use any include not listed in Section 1.
- Do not change the `pt()`, `addFlatTurf()`, `addSlopeTurf()`, `addWall()` signatures.
- Do not make a BUMP with a single `addSlopeTurf` call. A BUMP is always three calls.
- Do not pass surface Y as `cy`. Always subtract 0.1.
- Do not skip the end cap or the tee cap wall.
- Do not skip side wall coverage for any segment, including ramp and cliff transition zones.
- Do not place the cup before the obstacles. The cup is always the final block.
- Do not produce a file over 300 lines. Split first, then write.
- Do not add `std::cout` or any debug output.
- Do not use `glm::mat4` transforms inside the hole builder. All positions are computed arithmetically and passed to `pt()`.
