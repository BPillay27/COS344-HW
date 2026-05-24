#ifndef DRONE_H
#define DRONE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

#include "Figure.h"
#include "Camera.h"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

// Quadcopter drone modelled after a compact photography drone.
// Body: rectangular hull + top cover.
// Arms: 4 diagonal cylinders at 45-degree corners.
// Motors + propellers: at each arm tip.
// Landing gear: two skid runners with vertical posts.
// Camera gimbal: dark block with two lenses hanging below the nose.
//
// Local space: drone centred at origin, forward along -Z, up along +Y.
// World state: worldPosition + yaw/pitch/roll (degrees).
class Drone {
public:
    Drone();

    // Translation in drone-local directions.
    void moveForward(float delta);
    void moveRight(float delta);
    void moveUp(float delta);   // world-Y regardless of pitch/roll

    // Rotation (degrees per call, cumulative).
    void addYaw(float degrees);
    void addPitch(float degrees);
    void addRoll(float degrees);

    // Call once per frame. Spins propellers, updates the attached camera.
    void update(float deltaTime);

    // Upload all shape vertex data to the GPU. Call once after GLFW/GLEW init.
    void createGLBuffers();

    // Draw the drone. Sets the uMVP uniform in shaderID for each part group.
    void draw(GLuint shaderID, const glm::mat4& VP);

    const Camera& getCamera() const;
    glm::vec3 getPosition() const;
    AABB getAABB() const;

private:
    Figure staticParts;    // hull, arms, motors, landing gear, camera mount
    Figure props[4];       // propellers: FL, FR, BL, BR (spin separately)
    glm::vec3 propCenters[4]; // prop hub centres in drone-local space
    int propDirs[4];          // +1 or -1 to alternate CW / CCW spin

    glm::vec3 worldPosition;
    float yaw;    // degrees
    float pitch;  // degrees
    float roll;   // degrees
    float propAngle; // accumulated propeller rotation (degrees)

    Camera camera;

    static const float PROP_SPEED_DEG_PER_SEC;

    void assembleParts();
    void updateCamera();
    glm::mat4 buildModelMatrix() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    static void setMVP(GLuint shaderID, const glm::mat4& mvp);
    static glm::mat4 spinAroundPoint(const glm::vec3& centre, float angleDeg);
};

#endif // DRONE_H
