#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

using namespace glm;
using namespace std;

#include "shapes/Square.h"
#include "shapes/Cube.h"
#include "shapes/Cylinder.h"

#include "shapes/Circle.h"
#include "shapes/Cone.h"
#include "shapes/TriangularPrism.h"
#include "shapes/transformation.h"
#include "shapes/SquarePyramid.h"
#include "shapes/Sphere.h"
#include "shapes/light.h"

#include "Figure.h"
#include "Shape3D.h"
#include "RenderState.h"
#include "SpatialHash.h"

// Global variables
SpatialHash *gSpatialHash = nullptr;
Figure scene = Figure();

#if defined(__has_include)
#if __has_include("stb_image.h")
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#else
#include <cstddef>
static inline void stbi_set_flip_vertically_on_load(int) {}
static inline unsigned char *stbi_load(const char *, int *, int *, int *, int) { return nullptr; }
static inline void stbi_image_free(void *) {}
#endif
#else
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

bool wireframeMode = false;

struct KeyState
{
    bool W = false;
    bool A = false;
    bool S = false;
    bool D = false;
};

KeyState keyState;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_W)
            keyState.W = true;
        if (key == GLFW_KEY_A)
            keyState.A = true;
        if (key == GLFW_KEY_S)
            keyState.S = true;
        if (key == GLFW_KEY_D)
            keyState.D = true;
    }
    else if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_W)
            keyState.W = false;
        if (key == GLFW_KEY_A)
            keyState.A = false;
        if (key == GLFW_KEY_S)
            keyState.S = false;
        if (key == GLFW_KEY_D)
            keyState.D = false;
    }

    switch (key)
    {
    case GLFW_KEY_ENTER:
        if (action == GLFW_PRESS)
        {
            wireframeMode = !wireframeMode;
        }
        break;
    case GLFW_KEY_I:
        if (action == GLFW_PRESS)
        {
            gUseGrayscale = !gUseGrayscale;
        }
        break;
    }
}

const char *getError()
{
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW()
{
    glewExperimental = true;
    if (!glfwInit())
        throw getError();
}

inline void startUpGLEW()
{
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp()
{
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1000, 1000, "Experiment", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        throw "Failed to open GLFW window.\n";
    }
    glfwMakeContextCurrent(window);
    startUpGLEW();
    return window;
}

void renderObjects(GLuint programID, glm::mat4 view)
{
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(glGetUniformLocation(programID, "uView"), 1, GL_FALSE, glm::value_ptr(skyboxView));
    glDepthMask(GL_FALSE);

    // TODO: Render skybox here

    glDepthMask(GL_TRUE);
    glUniformMatrix4fv(glGetUniformLocation(programID, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    scene.draw();
}

int main()
{
    GLFWwindow *window;
    try
    {
        window = setUp();
    }
    catch (const char *e)
    {
        cout << e << endl;
        throw;
    }

    GLuint programID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    if (programID == 0)
    {
        std::cerr << "Failed to load shaders" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwSetKeyCallback(window, keyCallback);

    GLint locNM = glGetUniformLocation(programID, "uNormalMatrix");
    GLint locView = glGetUniformLocation(programID, "uView");
    GLint locModel = glGetUniformLocation(programID, "uModel");
    GLint locProjection = glGetUniformLocation(programID, "uProjection");

    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 orthogonalProjection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.1f, 100.0f); // Expanded size slightly

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 7.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 model = glm::mat4(1.0f);

    glUseProgram(programID);

    // ===== INITIALIZE LIGHTING UNIFORMS TO PREVENT SHADER REJECTION (CRITICAL) =====
    glUniform3f(glGetUniformLocation(programID, "uLightDir"), 0.5f, 1.0f, 0.4f);
    glUniform3f(glGetUniformLocation(programID, "uLightColor"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(programID, "uLightIntensity"), 0.8f);

    glUniform3f(glGetUniformLocation(programID, "uPointLightPos"), 0.0f, 4.0f, -1.5f);
    glUniform3f(glGetUniformLocation(programID, "uPointLightColor"), 1.0f, 0.9f, 0.8f);
    glUniform1f(glGetUniformLocation(programID, "uPointLightIntensity"), 1.0f);
    glUniform1f(glGetUniformLocation(programID, "uPointLightRange"), 25.0f);
    glUniform1f(glGetUniformLocation(programID, "uShininess"), 32.0f);

    // Explicitly toggle features off since textures aren't being loaded yet
    glUniform1i(glGetUniformLocation(programID, "useColor"), 0);
    glUniform1i(glGetUniformLocation(programID, "useAlphaMap"), 0);
    glUniform1i(glGetUniformLocation(programID, "useDisplacement"), 0);

    float cameraSpeed = 0.07f;
    gSpatialHash = new SpatialHash(2.5f);

    // Add red prism shape
    // glm::vec4 frontCenter(0.0f, -0.50f, 1.0f, 1.0f);
    // Square<4> frontFace(frontCenter, 3.0f, 2.0f);

    // glm::vec4 backCenter(0.0f, -1.50f, 1.0f, 1.0f);
    // Square<4> backFace(backCenter, 3.0f, 2.0f);

    // Cube<4>* rectangularPrism = new Cube<4>(frontFace, backFace);
    // rectangularPrism->setColour(9, 139, 74);
    // scene.addShape(rectangularPrism);

    // int prismID = scene.getNumShapes() - 1;
    // glm::vec3 hashPosition((frontCenter.x + backCenter.x) / 2.0f, (frontCenter.y + backCenter.y) / 2.0f, (frontCenter.z + backCenter.z) / 2.0f);
    // if (gSpatialHash != nullptr) gSpatialHash->insert(prismID, hashPosition);

    /**********************HOLE 6*************************** */
    // Hole 6 - Lower green rectangular prism
    float halfWidth_6 = 1.8f;
    float halfHeight_6 = 0.20f;

    // Lower Y value = lower on screen/world
    float centreY_6 = -0.75f;

    float centreZ_6 = -0.1f;
    float depth_6 = 2.5f;

    // Front face
    glm::vec4 frontTL_6(-halfWidth_6, centreY_6 + halfHeight_6, centreZ_6, 1.0f);
    glm::vec4 frontTR_6(halfWidth_6, centreY_6 + halfHeight_6, centreZ_6, 1.0f);
    glm::vec4 frontBR_6(halfWidth_6, centreY_6 - halfHeight_6, centreZ_6, 1.0f);
    glm::vec4 frontBL_6(-halfWidth_6, centreY_6 - halfHeight_6, centreZ_6, 1.0f);

    Square<4> frontFace_6(frontTL_6, frontTR_6, frontBR_6, frontBL_6);

    // Back face
    glm::vec4 backTL_6(-halfWidth_6, centreY_6 + halfHeight_6, centreZ_6 + depth_6, 1.0f);
    glm::vec4 backTR_6(halfWidth_6, centreY_6 + halfHeight_6, centreZ_6 + depth_6, 1.0f);
    glm::vec4 backBR_6(halfWidth_6, centreY_6 - halfHeight_6, centreZ_6 + depth_6, 1.0f);
    glm::vec4 backBL_6(-halfWidth_6, centreY_6 - halfHeight_6, centreZ_6 + depth_6, 1.0f);

    Square<4> backFace_6(backTL_6, backTR_6, backBR_6, backBL_6);

    // Create prism
    Cube<4> *rectangularPrism_6 = new Cube<4>(frontFace_6, backFace_6);
    rectangularPrism_6->setColour(9, 139, 74); // Green
    scene.addShape(rectangularPrism_6);

    // Second rectangular prism for Hole 6 - rotated around Z axis

    float centreX2_6 = 2.4f;      // Move to the right
    float centreY2_6 = centreY_6; // Same Y level
    float centreZ2_6 = 2.0f;      // Middle of prism in Z
    float angleDeg_6 = 35.0f;     // Angle between the two prisms
    float angleRad_6 = angleDeg_6 * 3.14159265f / 180.0f;

    // Rotate around Y axis (so Y stays the same, angle is made in X-Z plane)
    auto rotateAroundY_6 = [&](float x, float z) -> glm::vec2
    {
        float dx = x - centreX2_6;
        float dz = z - centreZ2_6;

        float newX = dx * cos(angleRad_6) - dz * sin(angleRad_6);
        float newZ = dx * sin(angleRad_6) + dz * cos(angleRad_6);

        return glm::vec2(centreX2_6 + newX, centreZ2_6 + newZ);
    };

    // Local unrotated corners of an IDENTICAL prism
    float left2_6 = centreX2_6 - halfWidth_6;
    float right2_6 = centreX2_6 + halfWidth_6;
    float top2_6 = centreY2_6 + halfHeight_6;
    float bottom2_6 = centreY2_6 - halfHeight_6;
    float front2_6 = centreZ2_6 - depth_6 / 2.0f;
    float back2_6 = centreZ2_6 + depth_6 / 2.0f;

    // Rotate front-face XZ coordinates
    glm::vec2 frontTL_xz_6 = rotateAroundY_6(left2_6, front2_6);
    glm::vec2 frontTR_xz_6 = rotateAroundY_6(right2_6, front2_6);
    glm::vec2 frontBR_xz_6 = rotateAroundY_6(right2_6, front2_6);
    glm::vec2 frontBL_xz_6 = rotateAroundY_6(left2_6, front2_6);

    // Rotate back-face XZ coordinates
    glm::vec2 backTL_xz_6 = rotateAroundY_6(left2_6, back2_6);
    glm::vec2 backTR_xz_6 = rotateAroundY_6(right2_6, back2_6);
    glm::vec2 backBR_xz_6 = rotateAroundY_6(right2_6, back2_6);
    glm::vec2 backBL_xz_6 = rotateAroundY_6(left2_6, back2_6);

    // Front face
    glm::vec4 frontTL2_6(frontTL_xz_6.x, top2_6, frontTL_xz_6.y, 1.0f);
    glm::vec4 frontTR2_6(frontTR_xz_6.x, top2_6, frontTR_xz_6.y, 1.0f);
    glm::vec4 frontBR2_6(frontBR_xz_6.x, bottom2_6, frontBR_xz_6.y, 1.0f);
    glm::vec4 frontBL2_6(frontBL_xz_6.x, bottom2_6, frontBL_xz_6.y, 1.0f);

    Square<4> frontFace2_6(frontTL2_6, frontTR2_6, frontBR2_6, frontBL2_6);

    // Back face
    glm::vec4 backTL2_6(backTL_xz_6.x, top2_6, backTL_xz_6.y, 1.0f);
    glm::vec4 backTR2_6(backTR_xz_6.x, top2_6, backTR_xz_6.y, 1.0f);
    glm::vec4 backBR2_6(backBR_xz_6.x, bottom2_6, backBR_xz_6.y, 1.0f);
    glm::vec4 backBL2_6(backBL_xz_6.x, bottom2_6, backBL_xz_6.y, 1.0f);

    Square<4> backFace2_6(backTL2_6, backTR2_6, backBR2_6, backBL2_6);

    // Create second prism
    Cube<4> *rectangularPrism2_6 = new Cube<4>(frontFace2_6, backFace2_6);
    rectangularPrism2_6->setColour(9, 139, 74); // Green
    scene.addShape(rectangularPrism2_6);

    // Green sphere
    glm::vec4 sphereCenter_6(1.5f, -0.7f, 0.2f, 1.0f); // xyz

    float sphereRadius_6 = 0.35f;
    int sphereStacks_6 = 24;
    int sphereSlices_6 = 32;

    Sphere<4> *greenSphere_6 = new Sphere<4>(
        sphereCenter_6,
        sphereRadius_6,
        sphereStacks_6,
        sphereSlices_6);

    greenSphere_6->setColour(9, 139, 74); // Green
    scene.addShape(greenSphere_6);

    // Hole 6 barriers around the ends of the green rectangles
    // Barrier colour: (235, 183, 93)

    float barrierHalfWidth_6 = 0.12f; // Barrier thickness along X
    float barrierHalfHeight_6 = halfHeight_6 * 1.4f;
    float barrierDepth_6 = depth_6;
    float barrierY_6 = centreY_6;

    // ---------------------------------------------------------
    // Barrier for LEFT end of first green prism
    // ---------------------------------------------------------

    float leftBarrierX_6 = -halfWidth_6 - barrierHalfWidth_6;

    glm::vec4 leftBarrierFrontTL_6(leftBarrierX_6 - barrierHalfWidth_6, barrierY_6 + barrierHalfHeight_6, centreZ_6, 1.0f);
    glm::vec4 leftBarrierFrontTR_6(leftBarrierX_6 + barrierHalfWidth_6, barrierY_6 + barrierHalfHeight_6, centreZ_6, 1.0f);
    glm::vec4 leftBarrierFrontBR_6(leftBarrierX_6 + barrierHalfWidth_6, barrierY_6 - barrierHalfHeight_6, centreZ_6, 1.0f);
    glm::vec4 leftBarrierFrontBL_6(leftBarrierX_6 - barrierHalfWidth_6, barrierY_6 - barrierHalfHeight_6, centreZ_6, 1.0f);

    Square<4> leftBarrierFront_6(
        leftBarrierFrontTL_6,
        leftBarrierFrontTR_6,
        leftBarrierFrontBR_6,
        leftBarrierFrontBL_6);

    glm::vec4 leftBarrierBackTL_6(leftBarrierX_6 - barrierHalfWidth_6, barrierY_6 + barrierHalfHeight_6, centreZ_6 + barrierDepth_6, 1.0f);
    glm::vec4 leftBarrierBackTR_6(leftBarrierX_6 + barrierHalfWidth_6, barrierY_6 + barrierHalfHeight_6, centreZ_6 + barrierDepth_6, 1.0f);
    glm::vec4 leftBarrierBackBR_6(leftBarrierX_6 + barrierHalfWidth_6, barrierY_6 - barrierHalfHeight_6, centreZ_6 + barrierDepth_6, 1.0f);
    glm::vec4 leftBarrierBackBL_6(leftBarrierX_6 - barrierHalfWidth_6, barrierY_6 - barrierHalfHeight_6, centreZ_6 + barrierDepth_6, 1.0f);

    Square<4> leftBarrierBack_6(
        leftBarrierBackTL_6,
        leftBarrierBackTR_6,
        leftBarrierBackBR_6,
        leftBarrierBackBL_6);

    Cube<4> *leftBarrier_6 = new Cube<4>(leftBarrierFront_6, leftBarrierBack_6);
    leftBarrier_6->setColour(235, 183, 93);
    scene.addShape(leftBarrier_6);

    // Black cylinder
    glm::vec4 cylinderCenter_6(-0.9f, -0.73f, 1.0f, 1.0f);

    float cylinderRadius_6 = 0.25f;
    float cylinderHeight_6 = 0.4f;
    int cylinderResolution_6 = 32;

    // axis:
    // 0 = along X
    // 1 = along Y
    // 2 = along Z
    int cylinderAxis_6 = 1;

    Cylinder<4> *blackCylinder_6 = new Cylinder<4>(
        cylinderCenter_6,
        cylinderRadius_6,
        cylinderHeight_6,
        cylinderResolution_6,
        cylinderAxis_6);

    blackCylinder_6->setColour(0, 0, 0); // Black
    scene.addShape(blackCylinder_6);
    /*************************************************** */

    scene.createGLBuffers();

    do
    {
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        // ===== SMOOTH FIRST-PERSON WASD FLIGHT CONTROLS =====
        glm::vec3 forwardDir = glm::normalize(cameraTarget - cameraPos);
        glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, upVector));

        if (keyState.W)
        {
            cameraPos += cameraSpeed * forwardDir;
            cameraTarget += cameraSpeed * forwardDir;
        }
        if (keyState.S)
        {
            cameraPos -= cameraSpeed * forwardDir;
            cameraTarget -= cameraSpeed * forwardDir;
        }
        if (keyState.A)
        {
            cameraPos -= cameraSpeed * rightDir;
            cameraTarget -= cameraSpeed * rightDir;
        }
        if (keyState.D)
        {
            cameraPos += cameraSpeed * rightDir;
            cameraTarget += cameraSpeed * rightDir;
        }

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);

        // Feed the active camera position to vertex shader for specular calculations
        glUniform3f(glGetUniformLocation(programID, "uCameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        // ==================== 1. Render Main View ====================
        glViewport(0, 0, 1000, 1000);
        glClearColor(0.0f, 0.0f, 0.4f, 1.0f); // Main View: Deep Blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint grayscaleLoc = glGetUniformLocation(programID, "useGrayscale");
        if (grayscaleLoc != -1)
            glUniform1i(grayscaleLoc, gUseGrayscale ? 1 : 0);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
        glUniformMatrix3fv(locNM, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        // The vertex shader expects a combined Model-View-Projection matrix in `uMVP`.
        GLint mvpLoc = glGetUniformLocation(programID, "uMVP");
        if (mvpLoc >= 0)
        {
            glm::mat4 mvp = projection * view * model;
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        }

        renderObjects(programID, view);

        // ==================== 2. Render Mini-Map ====================
        glEnable(GL_SCISSOR_TEST);
        glScissor(750, 0, 250, 250);
        glViewport(750, 0, 250, 250);

        // Clear the mini-map viewport to a unique background color (Dark Slate)
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(orthogonalProjection));

        // Track player view position dynamically from overhead (Top-down view)
        glm::vec3 miniMapPos = glm::vec3(cameraPos.x, 15.0f, cameraPos.z);
        glm::vec3 miniMapTarget = glm::vec3(cameraPos.x, 0.0f, cameraPos.z);
        glm::mat4 miniMapView = glm::lookAt(miniMapPos, miniMapTarget, glm::vec3(0.0f, 0.0f, -1.0f));

        glm::mat3 miniMapNormal = glm::transpose(glm::inverse(glm::mat3(miniMapView * model)));
        glUniformMatrix3fv(locNM, 1, GL_FALSE, glm::value_ptr(miniMapNormal));
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(miniMapView));
        // Set uMVP for mini-map rendering as well
        GLint mvpLocMini = glGetUniformLocation(programID, "uMVP");
        if (mvpLocMini >= 0)
        {
            glm::mat4 miniMVP = orthogonalProjection * miniMapView * model;
            glUniformMatrix4fv(mvpLocMini, 1, GL_FALSE, glm::value_ptr(miniMVP));
        }

        renderObjects(programID, miniMapView);

        glDisable(GL_SCISSOR_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);

    if (gSpatialHash)
        delete gSpatialHash;
    glDeleteProgram(programID);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}