#include "Drone.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"
#include "Shapes/Sphere.h"
#include <cmath>

const float Drone::PROP_SPEED_DEG_PER_SEC = 720.0f;

Drone::Drone()
    : worldPosition(0.0f, 0.5f, 3.0f),
      yaw(0.0f), pitch(0.0f), roll(0.0f),
      propAngle(0.0f)
{
    propDirs[0] =  1;  // FL: counter-clockwise
    propDirs[1] = -1;  // FR: clockwise
    propDirs[2] = -1;  // BL: clockwise
    propDirs[3] =  1;  // BR: counter-clockwise

    // Prop hub centres in drone-local space (set during assembleParts).
    for (int i = 0; i < 4; ++i)
        propCenters[i] = glm::vec3(0.0f);

    assembleParts();
    updateCamera();
}

// ---------------------------------------------------------------------------
// Model assembly
// All coordinates are in drone-local space:
//   forward = -Z,  right = +X,  up = +Y,  origin = drone centre of mass.
// Real-world scale: 1 unit = 1 metre.  Drone wingspan ~0.75 m.
// ---------------------------------------------------------------------------
void Drone::assembleParts() {

    // --- Body hull (white) --------------------------------------------------
    // Main rectangular hull: 0.28 wide, 0.09 tall, 0.20 deep.
    {
        glm::vec4 c(0.0f, 0.0f, 0.0f, 1.0f);
        Cube<4>* hull = new Cube<4>(c, 0.09f, 0.28f, 0.20f);
        hull->setColour(225, 225, 228);
        staticParts.addShape(hull);
    }
    // Narrow top ridge (gives the body a slight curved look).
    {
        glm::vec4 c(0.0f, 0.055f, 0.0f, 1.0f);
        Cube<4>* ridge = new Cube<4>(c, 0.025f, 0.18f, 0.14f);
        ridge->setColour(235, 235, 238);
        staticParts.addShape(ridge);
    }

    // --- Arms (4 diagonal cylinders, white) ---------------------------------
    // Arm tips are at (±0.34, 0, ±0.34) — 45-degree diagonals.
    // Arms start at the body corners (±0.13, 0, ±0.09).
    const float ARM_RADIUS  = 0.030f;
    const int   ARM_SEG     = 12;

    struct ArmDef { glm::vec4 start; glm::vec4 end; };
    ArmDef arms[4] = {
        { glm::vec4(-0.13f, 0.0f, -0.09f, 1.0f), glm::vec4(-0.34f, 0.0f, -0.34f, 1.0f) }, // FL
        { glm::vec4( 0.13f, 0.0f, -0.09f, 1.0f), glm::vec4( 0.34f, 0.0f, -0.34f, 1.0f) }, // FR
        { glm::vec4(-0.13f, 0.0f,  0.09f, 1.0f), glm::vec4(-0.34f, 0.0f,  0.34f, 1.0f) }, // BL
        { glm::vec4( 0.13f, 0.0f,  0.09f, 1.0f), glm::vec4( 0.34f, 0.0f,  0.34f, 1.0f) }, // BR
    };

    for (int i = 0; i < 4; ++i) {
        Cylinder<4>* arm = new Cylinder<4>(arms[i].start, arms[i].end, ARM_RADIUS, ARM_SEG);
        arm->setColour(220, 220, 223);
        staticParts.addShape(arm);
    }

    // --- Motors (short cylinders sitting on top of each arm tip, white) -----
    // Axis 1 = Y-axis (vertical cylinder).
    const float MOTOR_RADIUS = 0.035f;
    const float MOTOR_HEIGHT = 0.045f;
    const int   MOTOR_SEG    = 14;

    glm::vec3 tipXZ[4] = {
        glm::vec3(-0.34f, 0.0f, -0.34f),
        glm::vec3( 0.34f, 0.0f, -0.34f),
        glm::vec3(-0.34f, 0.0f,  0.34f),
        glm::vec3( 0.34f, 0.0f,  0.34f),
    };

    for (int i = 0; i < 4; ++i) {
        glm::vec4 mc(tipXZ[i].x, MOTOR_HEIGHT * 0.5f, tipXZ[i].z, 1.0f);
        Cylinder<4>* motor = new Cylinder<4>(mc, MOTOR_RADIUS, MOTOR_HEIGHT, MOTOR_SEG, 1);
        motor->setColour(60, 60, 65);
        staticParts.addShape(motor);
    }

    // --- Propellers (two crossed thin blade cubes per motor) ----------------
    // Each prop is two flat Cube blades arranged in a + cross, sitting just
    // above the motor top.  Alternating pairs are orange vs dark grey to
    // match a real photography drone.
    const float PROP_LENGTH = 0.38f;   // full span tip-to-tip
    const float PROP_CHORD  = 0.048f;  // blade width (chord)
    const float PROP_THICK  = 0.009f;  // blade thickness
    const float PROP_Y      = MOTOR_HEIGHT + 0.010f;

    int propColors[4][3] = {
        { 50,  50,  54},   // FL: dark grey
        {185,  75,  15},   // FR: orange
        {185,  75,  15},   // BL: orange
        { 50,  50,  54},   // BR: dark grey
    };

    for (int i = 0; i < 4; ++i) {
        propCenters[i] = glm::vec3(tipXZ[i].x, PROP_Y, tipXZ[i].z);
        glm::vec4 pc(propCenters[i].x, propCenters[i].y, propCenters[i].z, 1.0f);

        // Blade A: runs along the X axis
        Cube<4>* bladeA = new Cube<4>(pc, PROP_THICK, PROP_LENGTH, PROP_CHORD);
        bladeA->setColour(propColors[i][0], propColors[i][1], propColors[i][2]);
        props[i].addShape(bladeA);

        // Blade B: runs along the Z axis (perpendicular to A)
        Cube<4>* bladeB = new Cube<4>(pc, PROP_THICK, PROP_CHORD, PROP_LENGTH);
        bladeB->setColour(propColors[i][0], propColors[i][1], propColors[i][2]);
        props[i].addShape(bladeB);

        // Small hub cap in the centre, stays with the spinning figure
        glm::vec4 hc(pc.x, pc.y + 0.008f, pc.z, 1.0f);
        Cylinder<4>* hub = new Cylinder<4>(hc, 0.018f, 0.014f, 10, 1);
        hub->setColour(28, 28, 32);
        props[i].addShape(hub);
    }

    // --- Landing gear -------------------------------------------------------
    // Two skid runners (front-to-back) + four vertical posts.
    const float GEAR_RADIUS = 0.012f;
    const int   GEAR_SEG    = 8;
    const float SKID_Y      = -0.10f;
    const float POST_TOP_Y  = -0.045f;

    // Left runner
    {
        glm::vec4 c1(-0.09f, SKID_Y, -0.10f, 1.0f);
        glm::vec4 c2(-0.09f, SKID_Y,  0.10f, 1.0f);
        Cylinder<4>* runner = new Cylinder<4>(c1, c2, GEAR_RADIUS, GEAR_SEG);
        runner->setColour(215, 215, 218);
        staticParts.addShape(runner);
    }
    // Right runner
    {
        glm::vec4 c1( 0.09f, SKID_Y, -0.10f, 1.0f);
        glm::vec4 c2( 0.09f, SKID_Y,  0.10f, 1.0f);
        Cylinder<4>* runner = new Cylinder<4>(c1, c2, GEAR_RADIUS, GEAR_SEG);
        runner->setColour(215, 215, 218);
        staticParts.addShape(runner);
    }
    // Four vertical posts connecting runners to hull
    float postX[4] = { -0.09f, -0.09f,  0.09f,  0.09f };
    float postZ[4] = { -0.08f,  0.08f, -0.08f,  0.08f };
    for (int i = 0; i < 4; ++i) {
        glm::vec4 top(postX[i], POST_TOP_Y, postZ[i], 1.0f);
        glm::vec4 bot(postX[i], SKID_Y,     postZ[i], 1.0f);
        Cylinder<4>* post = new Cylinder<4>(top, bot, GEAR_RADIUS, GEAR_SEG);
        post->setColour(215, 215, 218);
        staticParts.addShape(post);
    }

    // --- Camera gimbal (dark block below the nose) -------------------------
    {
        glm::vec4 cc(0.0f, -0.07f, -0.09f, 1.0f);
        Cube<4>* mount = new Cube<4>(cc, 0.055f, 0.075f, 0.055f);
        mount->setColour(28, 28, 32);
        staticParts.addShape(mount);
    }
    // Left lens
    {
        glm::vec4 lc(-0.022f, -0.072f, -0.120f, 1.0f);
        Sphere<4>* lens = new Sphere<4>(lc, 0.014f, 8, 12);
        lens->setColour(12, 12, 22);
        staticParts.addShape(lens);
    }
    // Right lens
    {
        glm::vec4 rc( 0.022f, -0.072f, -0.120f, 1.0f);
        Sphere<4>* lens = new Sphere<4>(rc, 0.014f, 8, 12);
        lens->setColour(12, 12, 22);
        staticParts.addShape(lens);
    }
}

// ---------------------------------------------------------------------------
// GL buffer management
// ---------------------------------------------------------------------------

void Drone::createGLBuffers() {
    staticParts.createGLBuffers();
    for (int i = 0; i < 4; ++i)
        props[i].createGLBuffers();
}

// ---------------------------------------------------------------------------
// Per-frame update
// ---------------------------------------------------------------------------

void Drone::update(float deltaTime) {
    propAngle += PROP_SPEED_DEG_PER_SEC * deltaTime;
    if (propAngle >= 360.0f) propAngle -= 360.0f;
    updateCamera();
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void Drone::setMVP(GLuint shaderID, const glm::mat4& mvp) {
    GLint loc = glGetUniformLocation(shaderID, "uMVP");
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, &mvp[0][0]);
}

glm::mat4 Drone::spinAroundPoint(const glm::vec3& centre, float angleDeg) {
    glm::mat4 T    = glm::translate(glm::mat4(1.0f),  centre);
    glm::mat4 R    = glm::rotate   (glm::mat4(1.0f), glm::radians(angleDeg), glm::vec3(0,1,0));
    glm::mat4 Tinv = glm::translate(glm::mat4(1.0f), -centre);
    return T * R * Tinv;
}

void Drone::draw(GLuint shaderID, const glm::mat4& VP) {
    glm::mat4 model = buildModelMatrix();

    // Static hull
    setMVP(shaderID, VP * model);
    staticParts.draw();

    // Spinning propellers (separate MVP per prop)
    for (int i = 0; i < 4; ++i) {
        glm::mat4 spin    = spinAroundPoint(propCenters[i], propAngle * propDirs[i]);
        setMVP(shaderID, VP * model * spin);
        props[i].draw();
    }
}

// ---------------------------------------------------------------------------
// 6-DOF movement
// ---------------------------------------------------------------------------

// W/S: always move horizontally in the yaw direction, ignoring pitch and roll.
// This keeps altitude constant so Space/Shift are the only way to go up/down,
// making all three translation axes clearly distinct for the rubric demo.
void Drone::moveForward(float delta) {
    glm::vec3 fwd = getForward();
    fwd.y = 0.0f;
    float len = glm::length(fwd);
    if (len > 0.001f) fwd /= len;   // re-normalise after zeroing Y
    worldPosition += fwd * delta;
}

// A/D: horizontal strafe, also ignoring roll so it stays in the XZ plane.
void Drone::moveRight(float delta) {
    glm::vec3 right = getRight();
    right.y = 0.0f;
    float len = glm::length(right);
    if (len > 0.001f) right /= len;
    worldPosition += right * delta;
}

// Space/Shift: pure world-Y movement, completely independent of orientation.
void Drone::moveUp(float delta) {
    worldPosition.y += delta;
}

void Drone::addYaw(float degrees) {
    yaw += degrees;
    if (yaw >  360.0f) yaw -= 360.0f;
    if (yaw < -360.0f) yaw += 360.0f;
}

void Drone::addPitch(float degrees) {
    pitch += degrees;
    // Clamp to avoid gimbal lock at extreme angles.
    if (pitch >  85.0f) pitch =  85.0f;
    if (pitch < -85.0f) pitch = -85.0f;
}

void Drone::addRoll(float degrees) {
    roll += degrees;
    if (roll >  360.0f) roll -= 360.0f;
    if (roll < -360.0f) roll += 360.0f;
}

// ---------------------------------------------------------------------------
// Orientation helpers
// ---------------------------------------------------------------------------

glm::mat4 Drone::buildModelMatrix() const {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, worldPosition);
    m = glm::rotate(m, glm::radians(yaw),   glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(pitch), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(roll),  glm::vec3(0, 0, 1));
    return m;
}

glm::vec3 Drone::getForward() const {
    glm::mat4 m = buildModelMatrix();
    return glm::normalize(glm::vec3(m * glm::vec4(0, 0, -1, 0)));
}

glm::vec3 Drone::getRight() const {
    glm::mat4 m = buildModelMatrix();
    return glm::normalize(glm::vec3(m * glm::vec4(1, 0, 0, 0)));
}

glm::vec3 Drone::getUp() const {
    glm::mat4 m = buildModelMatrix();
    return glm::normalize(glm::vec3(m * glm::vec4(0, 1, 0, 0)));
}

// ---------------------------------------------------------------------------
// Camera
// Camera hangs below the nose, looking along the drone's forward axis.
// ---------------------------------------------------------------------------

void Drone::updateCamera() {
    glm::mat4 model = buildModelMatrix();

    // Third-person chase camera: 1.8 m behind the drone, 0.35 m above
    // centre, looking slightly ahead so the body fills the lower frame.
    glm::vec4 localEye   ( 0.0f,  0.35f,  1.8f, 1.0f);
    glm::vec4 localTarget( 0.0f,  0.05f, -1.0f, 1.0f);
    glm::vec4 localUp    ( 0.0f,  1.0f,   0.0f, 0.0f);

    glm::vec3 eye    = glm::vec3(model * localEye);
    glm::vec3 target = glm::vec3(model * localTarget);
    glm::vec3 up     = glm::normalize(glm::vec3(model * localUp));

    camera.setPosition(eye);
    camera.setTarget(target);
    camera.setUp(up);
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

const Camera& Drone::getCamera() const {
    return camera;
}

glm::vec3 Drone::getPosition() const {
    return worldPosition;
}

AABB Drone::getAABB() const {
    // Axis-aligned bounding box in world space (approximate, ignores rotation).
    // Wingspan ~0.75 m, height ~0.30 m (including landing gear).
    const glm::vec3 half(0.40f, 0.20f, 0.40f);
    return { worldPosition - half, worldPosition + half };
}
