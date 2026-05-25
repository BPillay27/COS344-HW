#include "Hole13.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Sphere.h"

static glm::vec4 pt13(float x, float y, float z, glm::vec3 off) {
    return glm::vec4(x + off.x, y + off.y, z + off.z, 1.0f);
}

void buildHole13(Figure& scene, glm::vec3 off) {
    const int TURF_R = 9, TURF_G = 170, TURF_B = 75;
    const int BORDER_R = 87, BORDER_G = 19, BORDER_B = 6;

    Cube<4>* base = new Cube<4>(pt13(0.0f, 0.0f, 0.0f, off), 0.18f, 3.6f, 0.10f);
    base->setColour(140, 140, 140, 1.0f);
    scene.addShape(base);

    Cube<4>* grass = new Cube<4>(pt13(0.0f, 0.08f, 0.0f, off), 0.06f, 3.2f, 0.03f);
    grass->setColour(TURF_R, TURF_G, TURF_B, 1.0f);
    scene.addShape(grass);

    Cube<4>* topBorder = new Cube<4>(pt13(0.0f, 0.55f, 0.0f, off), 0.08f, 3.6f, 0.02f);
    topBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(topBorder);

    Cube<4>* bottomBorder = new Cube<4>(pt13(0.0f, -0.38f, 0.0f, off), 0.08f, 3.6f, 0.02f);
    bottomBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(bottomBorder);

    Cube<4>* leftBorder = new Cube<4>(pt13(-1.72f, 0.08f, 0.0f, off), 0.93f, 0.08f, 0.02f);
    leftBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(leftBorder);

    Cube<4>* rightBorder = new Cube<4>(pt13(1.72f, 0.08f, 0.0f, off), 0.93f, 0.08f, 0.02f);
    rightBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(rightBorder);

    Cube<4>* startBlock = new Cube<4>(pt13(-1.15f, 0.14f, 0.0f, off), 0.42f, 0.22f, 0.04f);
    startBlock->setColour(101, 67, 33, 1.0f);
    scene.addShape(startBlock);

    Cube<4>* midBlock = new Cube<4>(pt13(0.0f, 0.14f, 0.0f, off), 0.50f, 0.20f, 0.04f);
    midBlock->setColour(148, 40, 60, 1.0f);
    scene.addShape(midBlock);

    Cube<4>* endBlock = new Cube<4>(pt13(1.05f, 0.14f, 0.0f, off), 0.42f, 0.22f, 0.04f);
    endBlock->setColour(101, 67, 33, 1.0f);
    scene.addShape(endBlock);

    Cylinder<4>* hole = new Cylinder<4>(pt13(1.35f, -0.08f, 0.0f, off), 0.05f, 0.12f, 32, 1);
    hole->setColour(0, 0, 0, 1.0f);
    scene.addShape(hole);

    Sphere<4>* golfBall = new Sphere<4>(pt13(-1.35f, -0.02f, 0.0f, off), 0.03f, 9, 12);
    golfBall->setColour(255, 255, 255, 1.0f);
    scene.addShape(golfBall);

    return;

    const float change = -0.25f;
    const float bBorderRx = 0.875f;
    const float topx = bBorderRx - (0.875f - 0.165f) + change;
#if 0
    const float topy = 0.005f;
    const float lowery = -0.5f;
    const float botx = -0.08f + change;

    const int TURF_R = 9, TURF_G = 170, TURF_B = 75;
    const int WATER_R = 10, WATER_G = 54, WATER_B = 125;
    const int BORDER_R = 87, BORDER_G = 19, BORDER_B = 6;
    const int OBSTACLE_R = 148, OBSTACLE_G = 40, OBSTACLE_B = 60;
    const int GOLD_R = 214, GOLD_G = 169, GOLD_B = 21;

    // Base Turf
    Cube<4>* base = new Cube<4>(pt13(0.0f, 0.0f, 0.0f, off), 1.8f, 1.98f, 0.12f);
    base->setColour(140, 140, 140, 1.0f);
    scene.addShape(base);

    // Broad fairway layer so the grass is unmistakable in the scene.
    Cube<4>* fairway = new Cube<4>(pt13(0.0f, 0.16f, 0.0f, off), 0.22f, 1.72f, 0.03f);
    fairway->setColour(TURF_R, TURF_G, TURF_B, 1.0f);
    scene.addShape(fairway);

    // Grass
    Cube<4>* grass = new Cube<4>(pt13(0.3775f, 0.3495f, 0.0f, off), 0.701f, 0.985f, 0.03f);
    grass->setColour(TURF_R, TURF_G, TURF_B, 1.0f);
    scene.addShape(grass);

    // Diagonal grass
    Cube<4>* dgrass = new Cube<4>(pt13(-0.5375f, 0.0975f, 0.0f, off), 1.195f, 0.805f, 0.03f);
    dgrass->setColour(TURF_R, TURF_G, TURF_B, 1.0f);
    scene.addShape(dgrass);

    // River
    Cube<4>* river = new Cube<4>(pt13(0.325f, 0.3495f, 0.0f, off), 0.701f, 0.22f, 0.03f);
    river->setColour(WATER_R, WATER_G, WATER_B, 1.0f);
    scene.addShape(river);

    // Bottom Corner
    Cube<4>* bottomCorner = new Cube<4>(pt13(botx - 0.005f, -0.385f, 0.0f, off), 0.20f, 0.25f, 0.02f);
    bottomCorner->setColour(GOLD_R, GOLD_G, GOLD_B, 1.0f);
    scene.addShape(bottomCorner);

    // Top Corner
    Cube<4>* topCorner = new Cube<4>(pt13(-0.829f, -0.3975f, 0.0f, off), 0.145f, 0.09f, 0.02f);
    topCorner->setColour(GOLD_R, GOLD_G, GOLD_B, 1.0f);
    scene.addShape(topCorner);

    // Top Border
    Cube<4>* tBorder = new Cube<4>(pt13(-0.1375f, 0.686f, 0.0f, off), 0.028f, 1.175f, 0.02f);
    tBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(tBorder);

    // Bottom Border
    Cube<4>* bBorder = new Cube<4>(pt13((topx + bBorderRx) * 0.5f, -0.2375f, 0.0f, off), 0.03f, (bBorderRx - topx), 0.02f);
    bBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(bBorder);

    // Lower Bottom Border
    Cube<4>* lbBorder = new Cube<4>(pt13((-0.875f + botx) * 0.5f, lowery + 0.015f, 0.0f, off), 0.03f, (botx - (-0.875f)), 0.02f);
    lbBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(lbBorder);

    // Diagonal Border
    Cube<4>* dBorder = new Cube<4>(pt13((topx + botx) * 0.5f, (topy + lowery) * 0.5f, 0.0f, off), (topy - lowery), (topx - botx), 0.02f);
    dBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(dBorder);

    // Left Border
    Cube<4>* lBorder = new Cube<4>(pt13(-0.5925f, (topy + lowery) * 0.5f, 0.0f, off), (topy - lowery), 0.64f, 0.02f);
    lBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(lBorder);

    // Right Border
    Cube<4>* rBorder = new Cube<4>(pt13(0.8645f, 0.3375f, 0.0f, off), 0.725f, 0.029f, 0.02f);
    rBorder->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(rBorder);

    // Outer frame to close the course boundaries cleanly.
    Cube<4>* frameTop = new Cube<4>(pt13(0.0f, 0.725f, 0.0f, off), 0.08f, 1.98f, 0.025f);
    frameTop->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(frameTop);

    Cube<4>* frameBottom = new Cube<4>(pt13(0.0f, -0.535f, 0.0f, off), 0.08f, 1.98f, 0.025f);
    frameBottom->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(frameBottom);

    Cube<4>* frameLeft = new Cube<4>(pt13(-0.955f, 0.095f, 0.0f, off), 0.74f, 0.08f, 0.025f);
    frameLeft->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(frameLeft);

    Cube<4>* frameRight = new Cube<4>(pt13(0.955f, 0.095f, 0.0f, off), 0.74f, 0.08f, 0.025f);
    frameRight->setColour(BORDER_R, BORDER_G, BORDER_B, 1.0f);
    scene.addShape(frameRight);

    // Bridge
    Cube<4>* bridge = new Cube<4>(pt13(0.3225f, 0.1875f, 0.0f, off), 0.325f, 0.245f, 0.04f);
    bridge->setColour(80, 100, 16, 1.0f);
    scene.addShape(bridge);

    // Start
    Cube<4>* start = new Cube<4>(pt13(0.775f, 0.3605f, 0.0f, off), 0.599f, 0.15f, 0.04f);
    start->setColour(128, 0, 1, 1.0f);
    scene.addShape(start);

    // Box
    Cube<4>* box = new Cube<4>(pt13(0.4605f, 0.375f, 0.0f, off), 0.15f, 0.045f, 0.04f);
    box->setColour(148, 40, 60, 1.0f);
    scene.addShape(box);

    // Hole
    Cylinder<4>* hole = new Cylinder<4>(pt13(-0.35f, -0.22f, 0.0f, off), 0.045f, 0.12f, 50, 1);
    hole->setColour(0, 0, 0, 1.0f);
    scene.addShape(hole);

    // Golf ball
    Sphere<4>* golfBall = new Sphere<4>(pt13(0.65f, 0.1f, 0.0f, off), 0.03f, 9, 12);
    golfBall->setColour(255, 255, 255, 1.0f);
    scene.addShape(golfBall);

    // Round obstacle
    Sphere<4>* obstacle = new Sphere<4>(pt13(0.107f, 0.476f, 0.0f, off), 0.102f, 12, 18);
    obstacle->setColour(214, 127, 60, 1.0f);
    scene.addShape(obstacle);

    // Optional extra deflectors as 3D cubes.
    Figure* deflector1 = new Figure();
    Cube<4>* b1 = new Cube<4>(pt13(0.0f, 0.3f, 0.0f, off), 0.6f, 2.0f, 0.25f);
    b1->setColour(101, 67, 33, 1.0f);
    deflector1->addShape(b1);
    deflector1->rotateY(45);
    deflector1->move(off.x - 3.5f / 2.0f + 0.5f, off.y, off.z - 18.0f / 3.0f);
    scene.addObject(deflector1);

    Figure* deflector2 = new Figure();
    Cube<4>* b2 = new Cube<4>(pt13(0.0f, 0.3f, 0.0f, off), 0.6f, 2.0f, 0.25f);
    b2->setColour(101, 67, 33, 1.0f);
    deflector2->addShape(b2);
    deflector2->rotateY(-45);
    deflector2->move(off.x + 3.5f / 2.0f - 0.5f, off.y, off.z - 2.0f * 18.0f / 3.0f);
    scene.addObject(deflector2);
}

#endif

}

/****
 
    _________ {}--____{}__
             |      |

 * * */