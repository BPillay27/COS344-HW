#include "Hole4.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Sphere.h"

static glm::vec4 pt4(float x, float y, float z, glm::vec3 off) {
    return glm::vec4(x + off.x, y + off.y, z + off.z, 1.0f);
}

// Layout notes:
// - Top layer: two small cups (left/right). A short pipe comes out of the right cup
//   into the wall and connects down to the lower L-shaped chamber.
// - Bottom layer: an L-shaped floor (two rectangles). The actual cup is positioned
//   on the long arm of the L.

void buildHole4(Figure& scene, glm::vec3 off) {
    const int TURF_R = 9, TURF_G = 139, TURF_B = 74;
    const int BORDER_R = 101, BORDER_G = 67, BORDER_B = 33;

    // Group everything so we can scale the entire hole at once
    Figure* holeGroup = new Figure();

    // Top layer base and grass
    Cube<4>* topBase = new Cube<4>(pt4(0.0f, 0.35f, 0.0f, off), 0.10f, 1.6f, 0.60f);
    topBase->setColour(120, 120, 120, 1.0f);
    holeGroup->addShape(topBase);

    Cube<4>* topGrass = new Cube<4>(pt4(0.0f, 0.40f, 0.0f, off), 0.06f, 1.4f, 0.48f);
    topGrass->setColour(TURF_R, TURF_G, TURF_B, 1.0f);
    holeGroup->addShape(topGrass);

    // Two small cups on the top layer (left and right)
    Cylinder<4>* topLeftCup = new Cylinder<4>(pt4(-0.4f, 0.30f, 0.0f, off), 0.05f, 0.08f, 24, 1);
    topLeftCup->setColour(0,0,0,1.0f);
    holeGroup->addShape(topLeftCup);

    Cylinder<4>* topRightCup = new Cylinder<4>(pt4(0.4f, 0.30f, 0.0f, off), 0.05f, 0.08f, 24, 1);
    topRightCup->setColour(0,0,0,1.0f);
    holeGroup->addShape(topRightCup);

    // Short pipe from the right cup into the wall (horizontal cylinder)
    Cylinder<4>* pipeOut = new Cylinder<4>(pt4(0.75f, 0.30f, 0.0f, off), 0.03f, 0.35f, 16, 0);
    pipeOut->setColour(160,160,160,1.0f);
    // rotate to point along +X axis: Cylinder constructor creates cylinder along Z, so apply an object.
    Figure* pipeFig = new Figure();
    pipeFig->addShape(pipeOut);
    pipeFig->rotateY(90);
    pipeFig->move(off.x + 0.75f, off.y, off.z);
    holeGroup->addObject(pipeFig);

    // Connector dropping through the wall into lower layer (vertical short cylinder)
    Cylinder<4>* drop = new Cylinder<4>(pt4(1.05f, 0.05f, 0.0f, off), 0.035f, 0.18f, 12, 0);
    drop->setColour(160,160,160,1.0f);
    holeGroup->addShape(drop);

    // Lower L-shaped chamber: two rectangular prisms forming an L
    // Long arm (horizontal along +X)
    Cube<4>* lowerLong = new Cube<4>(pt4(0.8f, -0.25f, 0.0f, off), 0.12f, 1.6f, 0.50f);
    lowerLong->setColour(TURF_R, TURF_G, TURF_B,1.0f);
    holeGroup->addShape(lowerLong);

    // Short arm (vertical along +Z) attached at the left end of long arm
    Cube<4>* lowerShort = new Cube<4>(pt4(-0.0f, -0.25f, -0.55f, off), 0.12f, 0.50f, 0.60f);
    lowerShort->setColour(TURF_R, TURF_G, TURF_B,1.0f);
    holeGroup->addShape(lowerShort);

    // Borders around the lower L
    Cube<4>* lowerTopBorder = new Cube<4>(pt4(0.4f, -0.05f, 0.0f, off), 0.06f, 1.8f, 0.04f);
    lowerTopBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    holeGroup->addShape(lowerTopBorder);

    Cube<4>* lowerLeftBorder = new Cube<4>(pt4(-0.8f, -0.25f, -0.55f, off), 0.12f, 0.04f, 0.6f);
    lowerLeftBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    holeGroup->addShape(lowerLeftBorder);

    // Hole on the long side of the L (towards +X end)
    Cylinder<4>* lowerHole = new Cylinder<4>(pt4(1.4f, -0.30f, 0.0f, off), 0.06f, 0.12f, 32, 1);
    lowerHole->setColour(0,0,0,1.0f);
    // Orient the hole vertically (cylinder default axis is Z) by rotating it into Y.
    Figure* lowerHoleFig = new Figure();
    lowerHoleFig->addShape(lowerHole);
    lowerHoleFig->rotateX(-90); // map Z-axis cylinder to Y-axis (vertical)
    holeGroup->addObject(lowerHoleFig);

    // Pipe exit where drop connects into the lower chamber (small short pipe piece)
    Cylinder<4>* pipeIn = new Cylinder<4>(pt4(1.05f, -0.05f, 0.0f, off), 0.03f, 0.20f, 12, 0);
    pipeIn->setColour(160,160,160,1.0f);
    Figure* pipeInFig = new Figure();
    pipeInFig->addShape(pipeIn);
    pipeInFig->rotateY(90);
    pipeInFig->move(off.x + 1.05f, off.y - 0.05f, off.z);
    holeGroup->addObject(pipeInFig);

    // Small decorative obstacle near the lower hole
    Sphere<4>* rock = new Sphere<4>(pt4(0.95f, -0.05f, 0.18f, off), 0.08f, 10, 12);
    rock->setColour(160,110,60,1.0f);
    holeGroup->addShape(rock);

    // Scale the whole hole up by 150% to make it bigger
    holeGroup->zoom(150);

    // Add the grouped hole to the scene
    scene.addObject(holeGroup);
}
