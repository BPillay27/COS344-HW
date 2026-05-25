#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono> 
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

using namespace glm;
using namespace std;

#include "Shapes/Square.h"
#include "Shapes/Cube.h"
#include "Shapes/Cylinder.h"

#include "Shapes/Circle.h"
#include "Shapes/Cone.h"
#include "Shapes/TriangularPrism.h"
#include "Shapes/transformation.h"
#include "Shapes/SquarePyramid.h"
#include "Shapes/Sphere.h"
#include "Shapes/light.h"
#include "Shapes/Imported.h"

#include "Figure.h"
#include "Shape3D.h"
#include "RenderState.h"
#include "SpatialHash.h"
#include "Drone.h"
#include "Hole8.h"
#include "Hole9.h"
#include "Hole10.h"
#include "Hole13.h"
#include "Hole4.h"
#include "Camera.h"


// Global variables
SpatialHash* gSpatialHash = nullptr;
Figure scene = Figure();
Drone* gDrone = nullptr;
std::vector<AABB> gDroneCollisionBoxes;

static AABB makeAABB(const glm::vec3& minimum, const glm::vec3& maximum) {
    return { minimum, maximum };
}

static void addCollisionBox(const glm::vec3& minimum, const glm::vec3& maximum) {
    gDroneCollisionBoxes.push_back(makeAABB(minimum, maximum));
}

static bool intersects(const AABB& a, const AABB& b) {
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y &&
           a.min.z <= b.max.z && a.max.z >= b.min.z;
}

static bool droneCanOccupy(const AABB& candidate) {
    if (candidate.min.y < -0.05f) {
        return false;
    }

    for (const auto& box : gDroneCollisionBoxes) {
        if (intersects(candidate, box)) {
            return false;
        }
    }

    return true;
}


// Mouse state for drone look
double gLastMouseX = 0.0, gLastMouseY = 0.0;
bool gFirstMouse = true;
const float MOUSE_SENSITIVITY = 0.05f;

void cursor_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (gFirstMouse) {
        gLastMouseX = xpos;
        gLastMouseY = ypos;
        gFirstMouse = false;
        return;
    }
    float dx =  (float)(xpos - gLastMouseX) * MOUSE_SENSITIVITY;
    float dy =  (float)(gLastMouseY - ypos) * MOUSE_SENSITIVITY; // inverted: up = positive pitch
    gLastMouseX = xpos;
    gLastMouseY = ypos;
    if (gDrone) {
        gDrone->addYaw(dx);
        gDrone->addPitch(dy);
    }
}


#if defined(__has_include)
#  if __has_include("stb_image.h")
#    define STB_IMAGE_IMPLEMENTATION
#    include "stb_image.h"
#  else
#    include <cstddef>
    static inline void stbi_set_flip_vertically_on_load(int) {}
    static inline unsigned char* stbi_load(const char*, int*, int*, int*, int){ return nullptr; }
    static inline void stbi_image_free(void*) {}
#  endif
#else
#  define STB_IMAGE_IMPLEMENTATION
#  include "stb_image.h"
#endif

bool wireframeMode = false;

directionalLight gSunLight(
    glm::vec4(-0.5f, 1.0f, 0.25f, 0.0f),
    glm::vec4(1.0f, 0.98f, 0.9f, 1.0f),
    0.9f);
float gSunAngle = -0.9f;
float gTimeScale = 0.45f;

struct KeyState {
    bool W = false;
    bool A = false;
    bool S = false;
    bool D = false;
};

KeyState keyState;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) keyState.W = true;
        if (key == GLFW_KEY_A) keyState.A = true;
        if (key == GLFW_KEY_S) keyState.S = true;
        if (key == GLFW_KEY_D) keyState.D = true;
    }
    else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_W) keyState.W = false;
        if (key == GLFW_KEY_A) keyState.A = false;
        if (key == GLFW_KEY_S) keyState.S = false;
        if (key == GLFW_KEY_D) keyState.D = false;
    }
    
    switch (key) {
        case GLFW_KEY_ENTER:
            if (action == GLFW_PRESS) {
                wireframeMode = !wireframeMode;
            }
            break;
        case GLFW_KEY_LEFT_BRACKET:
            if (action == GLFW_PRESS) {
                gTimeScale = std::min(gTimeScale * 1.25f, 5.0f);
                std::cout << "Time scale increased to " << gTimeScale << std::endl;
            }
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            if (action == GLFW_PRESS) {
                gTimeScale = std::max(gTimeScale / 1.25f, 0.05f);
                std::cout << "Time scale decreased to " << gTimeScale << std::endl;
            }
            break;
        case GLFW_KEY_I:
            if (action == GLFW_PRESS) {
                gUseGrayscale = !gUseGrayscale;  
            }
            break;
    }
}

const char *getError() {
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW() {
    glewExperimental = true; 
    std::cerr << "Calling glfwInit()" << std::endl;
    if (!glfwInit()) {
        const char *error = getError();
        std::cerr << "glfwInit() failed: " << (error ? error : "<no error>") << std::endl;
        throw error;
    }
    std::cerr << "glfwInit() succeeded" << std::endl;
}

inline void startUpGLEW() {
    glewExperimental = true; 
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp() {
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);               
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    std::cerr << "Creating GLFW window" << std::endl;
    GLFWwindow *window = glfwCreateWindow(1000, 1000, "Experiment", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        std::cerr << "glfwCreateWindow() failed" << std::endl;
        throw "Failed to open GLFW window.\n";
    }
    std::cerr << "GLFW window created" << std::endl;
    glfwMakeContextCurrent(window); 
    startUpGLEW();
    return window;
}

void renderObjects(GLuint programID, glm::mat4 view) {
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(glGetUniformLocation(programID, "uView"), 1, GL_FALSE, glm::value_ptr(skyboxView));
    glDepthMask(GL_FALSE); 
    
    // TODO: Render skybox here
    
    glDepthMask(GL_TRUE);   
    glUniformMatrix4fv(glGetUniformLocation(programID, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    scene.draw();
}

int main() {
    GLFWwindow *window;
    try { window = setUp(); }
    catch (const char *e) { cout << e << endl; throw; }

    std::cout << "GLFW window created successfully" << std::endl;

    GLuint programID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    std::cout << "LoadShaders returned programID=" << programID << std::endl;
    if (programID == 0) { std::cerr << "Failed to load shaders" << std::endl; glfwTerminate(); return 1; }
   
    glfwSetKeyCallback(window, keyCallback);
    
    GLint locNM         = glGetUniformLocation(programID, "uNormalMatrix");
    GLint locView       = glGetUniformLocation(programID, "uView");
    GLint locModel      = glGetUniformLocation(programID, "uModel");
    GLint locProjection = glGetUniformLocation(programID, "uProjection");
    GLint locMVP        = glGetUniformLocation(programID, "uMVP");
    
    glEnable(GL_DEPTH_TEST);
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 orthogonalProjection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.1f, 100.0f); // Expanded size slightly
    
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 7.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 model = glm::mat4(1.0f);
    
    glUseProgram(programID);
    std::cout << "Using shader program " << programID << std::endl;
    
    // ===== INITIALIZE LIGHTING UNIFORMS TO PREVENT SHADER REJECTION (CRITICAL) =====
    glUniform3f(glGetUniformLocation(programID, "uLightDir"), 0.5f, 1.0f, 0.4f);
    glUniform3f(glGetUniformLocation(programID, "uLightColor"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(programID, "uLightIntensity"), 0.8f);
    
    glUniform3f(glGetUniformLocation(programID, "uPointLightPos"), 0.0f, 4.0f, -1.5f);
    glUniform3f(glGetUniformLocation(programID, "uPointLightColor"), 1.0f, 0.9f, 0.8f);
    glUniform1f(glGetUniformLocation(programID, "uPointLightIntensity"), 1.0f);
    glUniform1f(glGetUniformLocation(programID, "uPointLightRange"), 25.0f);
    glUniform1f(glGetUniformLocation(programID, "uShininess"), 32.0f);
    std::cout << "Sun controls: '[' speeds time up, ']' slows it down." << std::endl;

    // Explicitly toggle features off since textures aren't being loaded yet
    glUniform1i(glGetUniformLocation(programID, "useColor"), 0);
    glUniform1i(glGetUniformLocation(programID, "useAlphaMap"), 0);
    glUniform1i(glGetUniformLocation(programID, "useDisplacement"), 0);
    
    float cameraSpeed = 0.0023f;
    gSpatialHash = new SpatialHash(2.5f);

    std::cout << "Initialization complete, entering main loop" << std::endl;
    
    // Drone init
    gDrone = new Drone();
    gDrone->setCollisionTest(droneCanOccupy);
    gDrone->createGLBuffers();

    // Build Hole geometry and register it in the global scene.
    buildHole8(scene, glm::vec3(0.0f, 0.0f, 0.0f));
    buildHole9(scene, glm::vec3(20.0f, 0.0f, -10.0f));
    buildHole10(scene, glm::vec3(28.0f, 0.0f, 25.0f)); // Placed to the right of Hole 9
    buildHole13(scene, glm::vec3(0.0f, 0.0f, -6.0f)); // Keep Hole 13 centered and visible
    buildHole4(scene, glm::vec3(-20.0f, 0.0f, -10.0f)); // Placed to the left of Hole 9

    // Coarse collision volumes for the visible course walls and major blockers.
    // These are deliberately thin so the drone can still fly over the turf.
    // Hole 8
    addCollisionBox(glm::vec3(-2.00f, 0.00f, -16.75f), glm::vec3(-1.75f, 2.10f, 0.25f));
    addCollisionBox(glm::vec3( 1.75f, 0.00f, -16.75f), glm::vec3( 2.00f, 2.10f, 0.25f));
    addCollisionBox(glm::vec3(-1.75f, 0.00f, -0.25f),  glm::vec3( 1.75f, 0.65f, 0.25f));
    addCollisionBox(glm::vec3(-1.75f, 0.00f, -16.75f), glm::vec3( 1.75f, 0.65f, -16.25f));

    // Hole 9
    addCollisionBox(glm::vec3(19.75f, 0.00f, -10.25f), glm::vec3(20.25f, 0.65f, -9.75f));
    addCollisionBox(glm::vec3(19.75f, 0.00f, -11.95f), glm::vec3(20.25f, 1.65f, -8.05f));
    addCollisionBox(glm::vec3(26.70f, 0.00f, -8.35f),  glm::vec3(27.05f, 1.65f, -1.15f));
    addCollisionBox(glm::vec3(29.65f, 0.00f, -11.95f), glm::vec3(30.00f, 1.65f, -1.15f));
    addCollisionBox(glm::vec3(20.00f, 0.00f, -11.95f), glm::vec3(30.00f, 0.65f, -11.70f));
    addCollisionBox(glm::vec3(27.75f, 0.00f, -1.50f),  glm::vec3(30.00f, 1.65f, -1.15f));

    // Hole 10
    addCollisionBox(glm::vec3(26.00f, 0.00f,  6.90f),  glm::vec3(26.35f, 0.65f, 25.10f));
    addCollisionBox(glm::vec3(29.65f, 0.00f,  6.90f),  glm::vec3(30.00f, 0.65f, 25.10f));
    addCollisionBox(glm::vec3(26.00f, 0.00f,  6.90f),  glm::vec3(30.00f, 0.65f, 7.15f));
    addCollisionBox(glm::vec3(26.00f, 0.00f, 25.10f),  glm::vec3(30.00f, 0.65f, 25.35f));

    // Hole 13
    addCollisionBox(glm::vec3(-1.90f, -6.38f, -6.03f), glm::vec3( 1.90f, -5.45f, -5.97f));
    addCollisionBox(glm::vec3(-1.90f, -6.38f, -5.03f), glm::vec3( 1.90f, -5.45f, -4.97f));
    addCollisionBox(glm::vec3(-1.90f, -5.92f, -6.03f), glm::vec3(-1.62f, -5.05f, -5.97f));
    addCollisionBox(glm::vec3( 1.62f, -5.92f, -6.03f), glm::vec3( 1.90f, -5.05f, -5.97f));

    // Hole 4
    addCollisionBox(glm::vec3(-20.85f, 0.00f, -10.35f), glm::vec3(-20.00f, 0.55f, -9.35f));
    addCollisionBox(glm::vec3(-19.20f, 0.00f, -10.35f), glm::vec3(-18.95f, 0.55f, -9.35f));
    addCollisionBox(glm::vec3(-20.85f, -0.40f, -10.35f), glm::vec3(-18.95f, -0.12f, -9.35f));
    addCollisionBox(glm::vec3(-20.15f, -0.38f, -10.85f), glm::vec3(-18.40f, -0.12f, -10.25f));
    addCollisionBox(glm::vec3(-19.60f, -0.38f, -10.85f), glm::vec3(-19.35f, 0.05f, -9.90f));
    /*
    ====================================
    Course One 
    ===================================
    */
    Figure* turf_1 = new Figure();
    Figure* walls_1 = new Figure();
    // Add red prism shape
    float halfWidth_1 = 1.0f;   // Change this to make it wider or narrower
    float halfHeight_1 = -0.25f; // Change this to make it taller or shorter
    float centreZ_1 = -2.0f;
    float difference_1= 4.5f; // Distance between front and back faces of the prism

    // Front Face (Horizontal)
    glm::vec4 startTL(halfWidth_1-1.0f,  halfHeight_1, centreZ_1, 1.0f);
    glm::vec4 startTR( halfWidth_1,  halfHeight_1, centreZ_1, 1.0f);
    glm::vec4 startBL(halfWidth_1-1.0f, halfHeight_1-0.24f, centreZ_1, 1.0f);
    glm::vec4 startBR( halfWidth_1, halfHeight_1-0.24f, centreZ_1, 1.0f);
    Square<4> start = Square<4>(startTL, startTR, startBR, startBL);

    // Back Face (Farther rectangle to viewer: +0.025f)
    glm::vec4 startTL2(halfWidth_1-1.0f,  halfHeight_1, centreZ_1 + difference_1 , 1.0f);
    glm::vec4 startTR2( halfWidth_1,  halfHeight_1, centreZ_1 + difference_1 , 1.0f);
    glm::vec4 startBL2(halfWidth_1-1.0f, halfHeight_1-0.24f, centreZ_1 + difference_1 , 1.0f);
    glm::vec4 startBR2( halfWidth_1, halfHeight_1-0.24f, centreZ_1 + difference_1 , 1.0f);
    Square<4> start2 = Square<4>(startTL2, startTR2, startBR2, startBL2);

    Cube<4>* Turf = new Cube<4>(start, start2);
    Turf->setColour(9, 139, 74); // Green
    turf_1->addShape(Turf);

    // Three enclosing walls around the turf, leaving the -Z side open.
    const float wallThickness = 0.05f;
    const float wallGap = 0.0f;
    const float wallTopY_1 = halfHeight_1 + 0.10f;
    const float wallBottomY_1 = halfHeight_1 - 0.24f;
    const float wallColorR_1 = 235;
    const float wallColorG_1 = 183;
    const float wallColorB_1 = 93;
    const float extraWallHeight_1 = 0.15f;
    const float raisedWallTopY_1 = wallTopY_1 + extraWallHeight_1;
    
    // Left wall (outside the turf, x < 0 side)
    glm::vec4 leftWallFrontTL(-wallGap, wallTopY_1, centreZ_1, 1.0f);
    glm::vec4 leftWallFrontTR(-wallGap, wallTopY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 leftWallFrontBR(-wallGap, wallBottomY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 leftWallFrontBL(-wallGap, wallBottomY_1, centreZ_1, 1.0f);
    Square<4> leftWallFront(leftWallFrontTL, leftWallFrontTR, leftWallFrontBR, leftWallFrontBL);

    glm::vec4 leftWallBackTL(-(wallGap + wallThickness), wallTopY_1, centreZ_1, 1.0f);
    glm::vec4 leftWallBackTR(-(wallGap + wallThickness), wallTopY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 leftWallBackBR(-(wallGap + wallThickness), wallBottomY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 leftWallBackBL(-(wallGap + wallThickness), wallBottomY_1, centreZ_1, 1.0f);
    Square<4> leftWallBack(leftWallBackTL, leftWallBackTR, leftWallBackBR, leftWallBackBL);

    Cube<4>* leftWall = new Cube<4>(leftWallFront, leftWallBack);
    leftWall->setColour((int)wallColorR_1, (int)wallColorG_1, (int)wallColorB_1, 1.0f);
    walls_1->addShape(leftWall);

    // Right wall (outside the turf, x > 1 side)
    glm::vec4 rightWallFrontTL(1.0f + wallGap, wallTopY_1, centreZ_1, 1.0f);
    glm::vec4 rightWallFrontTR(1.0f + wallGap, wallTopY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 rightWallFrontBR(1.0f + wallGap, wallBottomY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 rightWallFrontBL(1.0f + wallGap, wallBottomY_1, centreZ_1, 1.0f);
    Square<4> rightWallFront(rightWallFrontTL, rightWallFrontTR, rightWallFrontBR, rightWallFrontBL);

    glm::vec4 rightWallBackTL(1.0f + wallGap + wallThickness, wallTopY_1, centreZ_1, 1.0f);
    glm::vec4 rightWallBackTR(1.0f + wallGap + wallThickness, wallTopY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 rightWallBackBR(1.0f + wallGap + wallThickness, wallBottomY_1, centreZ_1 + difference_1, 1.0f);
    glm::vec4 rightWallBackBL(1.0f + wallGap + wallThickness, wallBottomY_1, centreZ_1, 1.0f);
    Square<4> rightWallBack(rightWallBackTL, rightWallBackTR, rightWallBackBR, rightWallBackBL);

    Cube<4>* rightWall = new Cube<4>(rightWallFront, rightWallBack);
    rightWall->setColour((int)wallColorR_1, (int)wallColorG_1, (int)wallColorB_1, 1.0f);
    walls_1->addShape(rightWall);

    // Third wall on the -Z side, flush with side walls to avoid corner gaps
    glm::vec4 farWallFrontTL(-(wallGap + wallThickness), wallTopY_1, centreZ_1, 1.0f);
    glm::vec4 farWallFrontTR(1.0f + wallGap + wallThickness, wallTopY_1, centreZ_1, 1.0f);
    glm::vec4 farWallFrontBR(1.0f + wallGap + wallThickness, wallBottomY_1, centreZ_1, 1.0f);
    glm::vec4 farWallFrontBL(-(wallGap + wallThickness), wallBottomY_1, centreZ_1, 1.0f);
    Square<4> farWallFront(farWallFrontTL, farWallFrontTR, farWallFrontBR, farWallFrontBL);

    glm::vec4 farWallBackTL(-(wallGap + wallThickness), wallTopY_1, centreZ_1 - wallThickness, 1.0f);
    glm::vec4 farWallBackTR(1.0f + wallGap + wallThickness, wallTopY_1, centreZ_1 - wallThickness, 1.0f);
    glm::vec4 farWallBackBR(1.0f + wallGap + wallThickness, wallBottomY_1, centreZ_1 - wallThickness, 1.0f);
    glm::vec4 farWallBackBL(-(wallGap + wallThickness), wallBottomY_1, centreZ_1 - wallThickness, 1.0f);
    Square<4> farWallBack(farWallBackTL, farWallBackTR, farWallBackBR, farWallBackBL);

    Cube<4>* farWall = new Cube<4>(farWallFront, farWallBack);
    farWall->setColour((int)wallColorR_1, (int)wallColorG_1, (int)wallColorB_1, 1.0f);
    walls_1->addShape(farWall);
    // Horizontal cylinder from one side wall to the other.
    // Centered on the turf top so half of it is embedded in the turf.
    const float turfTopY_1 = halfHeight_1;
    const float turfMidZ_1 = centreZ_1 + (difference_1 * 0.5f);
    const float cylinderBackOffsetZ_1 = 0.0f; // keep cylinder centred over turf (no extra back offset)
    const float cylinderRadius_1 = 0.12f;      // larger radius
    const float cylinderLength_1 = 1.0f; // from x=0 wall to x=1 wall
    glm::vec4 cylinderCenter_1(0.5f, turfTopY_1, turfMidZ_1 + cylinderBackOffsetZ_1, 1.0f);
    Cylinder<4>* turfCylinder_1 = new Cylinder<4>(cylinderCenter_1, cylinderRadius_1, cylinderLength_1, 24, 0);
    turfCylinder_1->setColour(9, 139, 74);
    turf_1->addShape(turfCylinder_1);

    

    // Extra thin raised wall where the bump starts: make it a simple vertical plane (no lateral extrusion)
    const float bumpStartZ_1 = cylinderCenter_1[2] + cylinderRadius_1;
    // Create a thin wall with minimal thickness centered at bumpStartZ_1
        // (Removed extra raised bump wall as requested)

    // Extra raised wall: place it at the front (near) edge of the turf instead of the far edge
    const float frontEdgeZ_1 = centreZ_1; // front / near edge
    glm::vec4 backWallFrontTL_1(-(wallGap + wallThickness), raisedWallTopY_1, frontEdgeZ_1, 1.0f);
    glm::vec4 backWallFrontTR_1(1.0f + wallGap + wallThickness, raisedWallTopY_1, frontEdgeZ_1, 1.0f);
    glm::vec4 backWallFrontBR_1(1.0f + wallGap + wallThickness, wallBottomY_1, frontEdgeZ_1, 1.0f);
    glm::vec4 backWallFrontBL_1(-(wallGap + wallThickness), wallBottomY_1, frontEdgeZ_1, 1.0f);
    // Place the back face of this raised strip slightly toward -Z using the wall thickness
    glm::vec4 backWallBackTL_1(-(wallGap + wallThickness), raisedWallTopY_1, frontEdgeZ_1 - wallThickness, 1.0f);
    glm::vec4 backWallBackTR_1(1.0f + wallGap + wallThickness, raisedWallTopY_1, frontEdgeZ_1 - wallThickness, 1.0f);
    glm::vec4 backWallBackBR_1(1.0f + wallGap + wallThickness, wallBottomY_1, frontEdgeZ_1 - wallThickness, 1.0f);
    glm::vec4 backWallBackBL_1(-(wallGap + wallThickness), wallBottomY_1, frontEdgeZ_1 - wallThickness, 1.0f);

    Square<4> backWallFront_1(backWallFrontTL_1, backWallFrontTR_1, backWallFrontBR_1, backWallFrontBL_1);
    Square<4> backWallBack_1(backWallBackTL_1, backWallBackTR_1, backWallBackBR_1, backWallBackBL_1);

    Cube<4>* backWall_1 = new Cube<4>(backWallFront_1, backWallBack_1);
    backWall_1->setColour((int)wallColorR_1, (int)wallColorG_1, (int)wallColorB_1, 1.0f);
    walls_1->addShape(backWall_1);

    // --- Vertical hole cylinder at the raised front strip ---
    // Top of the hole should be 0.001f higher than the turf top
    const float turfBottomY_1 = halfHeight_1 - 0.24f;
    const float turfHeight_1 = turfTopY_1 - turfBottomY_1; // should be 0.24f
    const float holeExtra = 0.002f; // 0.001 above and 0.001 below turf
    const float holeScaleFactor_1 = 3.0f;
    const float holeHeight_1 = (turfHeight_1 + holeExtra) * holeScaleFactor_1;
    const float holeRadius_1 = 0.30f;
    const float holeCenterX_1 = 0.5f;
    // Move hole to turf centre (not in the front wall)
    // Place hole under the horizontal "dump" cylinder at the midpoint between its two previous Z placements
    const float prevCylinderBackOffsetZ_1 = -2.0f; // previous back offset used earlier
    const float holeCenterZ_1 = turfMidZ_1 + (prevCylinderBackOffsetZ_1 + cylinderBackOffsetZ_1) * 0.5f;
    // place top at turfTopY_1 + 0.001f, so center is that minus half height
    const float holeCenterY_1 = (turfTopY_1 + 0.001f) - (holeHeight_1 * 0.5f);
    glm::vec4 holeCenter_1(holeCenterX_1, holeCenterY_1, holeCenterZ_1, 1.0f);
    Cylinder<4>* turfHole_1 = new Cylinder<4>(holeCenter_1, holeRadius_1, holeHeight_1, 24, 1); // axis=1 => Y-axis (vertical)
    turfHole_1->setColour(0, 0, 0);
    Figure* holeFigure_1 = new Figure();
    holeFigure_1->addShape(turfHole_1);

    // --- Add longer raised strips on left and right walls that touch the raised front/back walls ---
    // Make them span from the raised front edge to the turf back edge so they connect to the back wall
    const float sideRaisedStartZ = frontEdgeZ_1; // start at the raised front strip
    const float sideRaisedEndZ = centreZ_1 + difference_1; // extend to the turf back edge

    // Left raised strip (on left wall x extents)
    glm::vec4 leftRaisedFrontTL(-(wallGap), raisedWallTopY_1, sideRaisedStartZ, 1.0f);
    glm::vec4 leftRaisedFrontTR(-(wallGap), raisedWallTopY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 leftRaisedFrontBR(-(wallGap), wallBottomY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 leftRaisedFrontBL(-(wallGap), wallBottomY_1, sideRaisedStartZ, 1.0f);

    glm::vec4 leftRaisedBackTL(-(wallGap + wallThickness), raisedWallTopY_1, sideRaisedStartZ, 1.0f);
    glm::vec4 leftRaisedBackTR(-(wallGap + wallThickness), raisedWallTopY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 leftRaisedBackBR(-(wallGap + wallThickness), wallBottomY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 leftRaisedBackBL(-(wallGap + wallThickness), wallBottomY_1, sideRaisedStartZ, 1.0f);

    Square<4> leftRaisedFront(leftRaisedFrontTL, leftRaisedFrontTR, leftRaisedFrontBR, leftRaisedFrontBL);
    Square<4> leftRaisedBack(leftRaisedBackTL, leftRaisedBackTR, leftRaisedBackBR, leftRaisedBackBL);
    Cube<4>* leftRaised = new Cube<4>(leftRaisedFront, leftRaisedBack);
    leftRaised->setColour((int)wallColorR_1, (int)wallColorG_1, (int)wallColorB_1, 1.0f);
    walls_1->addShape(leftRaised);

    // Right raised strip (on right wall x extents)
    glm::vec4 rightRaisedFrontTL(1.0f + wallGap, raisedWallTopY_1, sideRaisedStartZ, 1.0f);
    glm::vec4 rightRaisedFrontTR(1.0f + wallGap, raisedWallTopY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 rightRaisedFrontBR(1.0f + wallGap, wallBottomY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 rightRaisedFrontBL(1.0f + wallGap, wallBottomY_1, sideRaisedStartZ, 1.0f);

    glm::vec4 rightRaisedBackTL(1.0f + wallGap + wallThickness, raisedWallTopY_1, sideRaisedStartZ, 1.0f);
    glm::vec4 rightRaisedBackTR(1.0f + wallGap + wallThickness, raisedWallTopY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 rightRaisedBackBR(1.0f + wallGap + wallThickness, wallBottomY_1, sideRaisedEndZ, 1.0f);
    glm::vec4 rightRaisedBackBL(1.0f + wallGap + wallThickness, wallBottomY_1, sideRaisedStartZ, 1.0f);

    Square<4> rightRaisedFront(rightRaisedFrontTL, rightRaisedFrontTR, rightRaisedFrontBR, rightRaisedFrontBL);
    Square<4> rightRaisedBack(rightRaisedBackTL, rightRaisedBackTR, rightRaisedBackBR, rightRaisedBackBL);
    Cube<4>* rightRaised = new Cube<4>(rightRaisedFront, rightRaisedBack);
    rightRaised->setColour((int)wallColorR_1, (int)wallColorG_1, (int)wallColorB_1, 1.0f);
    walls_1->addShape(rightRaised);

    Imported<4>* dogModel = new Imported<4>("Models/Hole_1_Dog.glb");
    Shape3D* dogShape = new Shape3D(dogModel);
    dogShape->zoom(-80);
    float dogX = 0.5f;                               // Matches cylinderCenter_1 X
    float dogY = turfTopY_1 + cylinderRadius_1;      // Cylinder Center Y + Radius = Top of cylinder
    float dogZ = turfMidZ_1 + cylinderBackOffsetZ_1;

    dogShape->move(dogX, dogY, dogZ);
    Figure* dogFigure = new Figure();
    dogFigure->addShape3D(dogShape);


    holeFigure_1->addObject(turf_1);
    holeFigure_1->addObject(dogFigure);
    holeFigure_1->addObject(walls_1);

    holeFigure_1->move(-0.25f, 0.750f, 0.0f); // Adjust position to align with the turf and walls
    // TO MOVE HOLE ONE 

    scene.addObject(holeFigure_1);


    
    int prismID = scene.getNumShapes() - 1;
    glm::vec3 hashPosition(0.0f, 0.0f, centreZ_1 + 0.0125f);
    if (gSpatialHash != nullptr) gSpatialHash->insert(prismID, hashPosition);


    //******************************HOLE 3********************************************* */
    Figure *hole_3 = new Figure();

    float halfWidth_3 = 3.1f;
    float halfHeight_3 = 0.20f;

    // Move Hole 3 significantly to the right
    float centreX_3 = 9.0f;

    // Lower Y value = lower on screen/world
    float centreY_3 = -0.75f;

    float centreZ_3 = -0.1f;
    float depth_3 = 5.0f;

    // Front face
    glm::vec4 frontTL_3(centreX_3 - halfWidth_3, centreY_3 + halfHeight_3, centreZ_3, 1.0f);
    glm::vec4 frontTR_3(centreX_3 + halfWidth_3, centreY_3 + halfHeight_3, centreZ_3, 1.0f);
    glm::vec4 frontBR_3(centreX_3 + halfWidth_3, centreY_3 - halfHeight_3, centreZ_3, 1.0f);
    glm::vec4 frontBL_3(centreX_3 - halfWidth_3, centreY_3 - halfHeight_3, centreZ_3, 1.0f);

    Square<4> frontFace_3(frontTL_3, frontTR_3, frontBR_3, frontBL_3);

    // Back face
    glm::vec4 backTL_3(centreX_3 - halfWidth_3, centreY_3 + halfHeight_3, centreZ_3 + depth_3, 1.0f);
    glm::vec4 backTR_3(centreX_3 + halfWidth_3, centreY_3 + halfHeight_3, centreZ_3 + depth_3, 1.0f);
    glm::vec4 backBR_3(centreX_3 + halfWidth_3, centreY_3 - halfHeight_3, centreZ_3 + depth_3, 1.0f);
    glm::vec4 backBL_3(centreX_3 - halfWidth_3, centreY_3 - halfHeight_3, centreZ_3 + depth_3, 1.0f);

    Square<4> backFace_3(backTL_3, backTR_3, backBR_3, backBL_3);

    // Create prism
    Cube<4> *rectangularPrism_3 = new Cube<4>(frontFace_3, backFace_3);
    rectangularPrism_3->setColour(9, 139, 74); // Green
    hole_3->addShape(rectangularPrism_3);

    //------------------------------------------------------------------------------------------
    // Thin edge barriers around Hole 3 prism
    // Barrier colour: (235, 183, 93)

    float barrierThickness_3 = 0.12f;
    float barrierHeight_3 = 0.18f;

    // Place barriers slightly above the green prism
    float barrierBottomY_3 = centreY_3 + halfHeight_3;
    float barrierTopY_3 = barrierBottomY_3 + barrierHeight_3;

    // Helper lambda to create a rectangular prism using min/max bounds
    auto addBarrier_3 = [&](float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
    {
        // Front face at zMin
        glm::vec4 fTL(xMin, yMax, zMin, 1.0f);
        glm::vec4 fTR(xMax, yMax, zMin, 1.0f);
        glm::vec4 fBR(xMax, yMin, zMin, 1.0f);
        glm::vec4 fBL(xMin, yMin, zMin, 1.0f);

        Square<4> front(fTL, fTR, fBR, fBL);

        // Back face at zMax
        glm::vec4 bTL(xMin, yMax, zMax, 1.0f);
        glm::vec4 bTR(xMax, yMax, zMax, 1.0f);
        glm::vec4 bBR(xMax, yMin, zMax, 1.0f);
        glm::vec4 bBL(xMin, yMin, zMax, 1.0f);

        Square<4> back(bTL, bTR, bBR, bBL);

        Cube<4> *barrier = new Cube<4>(front, back);
        barrier->setColour(235, 183, 93);
        hole_3->addShape(barrier);
    };

    // Main prism bounds
    float prismLeft_3 = centreX_3 - halfWidth_3;
    float prismRight_3 = centreX_3 + halfWidth_3;
    float prismFrontZ_3 = centreZ_3;
    float prismBackZ_3 = centreZ_3 + depth_3;

    // Left edge barrier
    addBarrier_3(
        prismLeft_3 - barrierThickness_3,
        prismLeft_3,
        barrierBottomY_3,
        barrierTopY_3,
        prismFrontZ_3,
        prismBackZ_3);

    // Right edge barrier
    addBarrier_3(
        prismRight_3,
        prismRight_3 + barrierThickness_3,
        barrierBottomY_3,
        barrierTopY_3,
        prismFrontZ_3,
        prismBackZ_3);

    // Front edge barrier
    addBarrier_3(
        prismLeft_3,
        prismRight_3,
        barrierBottomY_3,
        barrierTopY_3,
        prismFrontZ_3 - barrierThickness_3,
        prismFrontZ_3);

    // Back edge barrier
    addBarrier_3(
        prismLeft_3,
        prismRight_3,
        barrierBottomY_3,
        barrierTopY_3,
        prismBackZ_3,
        prismBackZ_3 + barrierThickness_3);

    //---------------------------------------------------------------------------------------
    // Add Hole 3 to the main sceness

    glm::vec4 importedCentre_3(centreX_3, centreY_3 + 0.95f, centreZ_3 + 2.5f, 1.0f);

    Imported<4> *importedModel_3 = new Imported<4>(
        "Models/Hole_1_Dog.glb",
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), // load at origin first
        0.7f                               // scale
    );

    // Rotate while still at origin
    *importedModel_3 *= roty<4>(90);

    // Move to the final position you want
    *importedModel_3 *= translation<4>(
        importedCentre_3[0],
        importedCentre_3[1],
        importedCentre_3[2]);

    importedModel_3->setColour(9, 139, 74);
    hole_3->addShape(importedModel_3);

    //--------------------------------------------------
    glm::vec4 importedCentre_3_1(centreX_3 - 1.5f, centreY_3 + 0.38f, centreZ_3 + 0.85f, 1.0f);

    Imported<4> *importedModel_3_1 = new Imported<4>(
        "Models/Hole_3_Deformed_Rock.glb",
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), // load at origin first
        0.2f                               // scale
    );

    // Rotate while still at origin
    *importedModel_3_1 *= roty<4>(90);

    // Move to the final position you want
    *importedModel_3_1 *= translation<4>(
        importedCentre_3_1[0],
        importedCentre_3_1[1],
        importedCentre_3_1[2]);

    importedModel_3_1->setColour(9, 139, 74);
    hole_3->addShape(importedModel_3_1);

    //------------------------------------------------------
    glm::vec4 cylinderCenter_3(
        frontBL_3[0] + 0.3f,
        frontBL_3[1] + 0.0f,
        frontBL_3[2] + 0.8,
        1.0f);

    float cylinderRadius_3 = 0.20f;
    float cylinderHeight_3 = 0.8f;
    int cylinderResolution_3 = 32;

    // Axis:
    // 0 = along X
    // 1 = along Y
    // 2 = along Z
    int cylinderAxis_3 = 1;

    Cylinder<4> *blackCylinder_3 = new Cylinder<4>(
        cylinderCenter_3,
        cylinderRadius_3,
        cylinderHeight_3,
        cylinderResolution_3,
        cylinderAxis_3);

    blackCylinder_3->setColour(67, 0, 0); // Black
    hole_3->addShape(blackCylinder_3);

    scene.addObject(hole_3);

    //************************************************************************** */

    /**********************HOLE 6*************************** */
    // Hole 6 - Lower green rectangular prism
    Figure *hole_6 = new Figure();
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
    hole_6->addShape(rectangularPrism_6);

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
    hole_6->addShape(rectangularPrism2_6);

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
    hole_6->addShape(greenSphere_6);

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
    hole_6->addShape(leftBarrier_6);

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

    // blackCylinder_6->setColour(0, 0, 0); // Black
    // hole_6->addShape(blackCylinder_6);
    scene.addObject(hole_6);
    /*************************************************** */

    scene.createGLBuffers();
    std::cout << "Hole 8 geometry loaded. Program initialised successfully." << std::endl;
    std::cout << "Hole 9 geometry loaded. Program initialised successfully." << std::endl;
    std::cout << "Hole 10 geometry loaded. Program initialised successfully." << std::endl;

    float lastTime = (float)glfwGetTime();

    do {
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        // Delta time for drone animation
        float currentTime = (float)glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;
        if (dt > 0.1f) dt = 0.1f; // clamp to avoid jumps after stalls

        gSunAngle += gTimeScale * 0.35f * dt;
        glm::vec3 sunDir = glm::normalize(glm::vec3(
            cosf(gSunAngle),
            sinf(gSunAngle),
            0.35f));
        gSunLight.setDirection(glm::vec4(sunDir, 0.0f));
        if (gDrone) gDrone->update(dt);

        // ===== DRONE FLIGHT CONTROLS =====
        // W/S  : forward / back        A/D    : strafe left / right
        // Space: ascend                Shift  : descend
        // Q/E  : roll left / right     Mouse  : yaw and pitch (cursor callback)
        const float MOVE_SPEED = 5.0f;
        const float ROLL_SPEED = 90.0f;
        const float LOOK_SPEED = 60.0f; // degrees per second for arrow-key look
        if (gDrone) {
            if (glfwGetKey(window, GLFW_KEY_W)          == GLFW_PRESS) gDrone->moveForward( MOVE_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_S)          == GLFW_PRESS) gDrone->moveForward(-MOVE_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_A)          == GLFW_PRESS) gDrone->moveRight(-MOVE_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_D)          == GLFW_PRESS) gDrone->moveRight( MOVE_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_SPACE)      == GLFW_PRESS) gDrone->moveUp( MOVE_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) gDrone->moveUp(-MOVE_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_Q)          == GLFW_PRESS) gDrone->addRoll(-ROLL_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_E)          == GLFW_PRESS) gDrone->addRoll( ROLL_SPEED * dt);
            // Arrow keys: look around
            if (glfwGetKey(window, GLFW_KEY_LEFT)       == GLFW_PRESS) gDrone->addYaw(  LOOK_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_RIGHT)      == GLFW_PRESS) gDrone->addYaw(  -LOOK_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_UP)         == GLFW_PRESS) gDrone->addPitch( LOOK_SPEED * dt);
            if (glfwGetKey(window, GLFW_KEY_DOWN)       == GLFW_PRESS) gDrone->addPitch(-LOOK_SPEED * dt);
        }

        // Main view comes from the drone's onboard camera.
        glm::mat4 VP     = gDrone ? gDrone->getCamera().getViewProjection() : glm::mat4(1.0f);
        glm::mat4 V      = gDrone ? gDrone->getCamera().getViewMatrix()     : glm::mat4(1.0f);
        glm::vec3 camPos = gDrone ? gDrone->getCamera().getPosition()       : glm::vec3(0.0f);

        // Camera position for specular lighting in the vertex shader.
        glUniform3f(glGetUniformLocation(programID, "uCameraPos"), camPos.x, camPos.y, camPos.z);
        glUniform3f(glGetUniformLocation(programID, "uLightDir"), sunDir.x, sunDir.y, sunDir.z);
        glm::vec4 sunColor = gSunLight.getColor();
        glUniform3f(glGetUniformLocation(programID, "uLightColor"), sunColor.x, sunColor.y, sunColor.z);
        glUniform1f(glGetUniformLocation(programID, "uLightIntensity"), gSunLight.getIntensity());

        // ==================== 1. Render Main View ====================
        glViewport(0, 0, 1000, 1000);
        glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint grayscaleLoc = glGetUniformLocation(programID, "useGrayscale");
        if (grayscaleLoc != -1) glUniform1i(grayscaleLoc, gUseGrayscale ? 1 : 0);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(V * model)));
        glUniformMatrix3fv(locNM,         1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniformMatrix4fv(locModel,      1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(locView,       1, GL_FALSE, glm::value_ptr(V));

        // uMVP drives gl_Position in the vertex shader.
        glm::mat4 mvp = VP * model;
        if (locMVP >= 0) glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(mvp));

        renderObjects(programID, V);
        if (gDrone) gDrone->draw(programID, VP);

        // ==================== 2. Mini-Map (top-down, tracks drone) ====================
        glEnable(GL_SCISSOR_TEST);
        glScissor(750, 0, 250, 250);
        glViewport(750, 0, 250, 250);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 dronePos   = gDrone ? gDrone->getPosition() : glm::vec3(0.0f);
        glm::vec3 mmEye      = glm::vec3(dronePos.x, 15.0f, dronePos.z);
        glm::vec3 mmTarget   = glm::vec3(dronePos.x,  0.0f, dronePos.z);
        glm::mat4 miniMapView = glm::lookAt(mmEye, mmTarget, glm::vec3(0.0f, 0.0f, -1.0f));

        glm::mat3 miniNM = glm::transpose(glm::inverse(glm::mat3(miniMapView * model)));
        glUniformMatrix3fv(locNM,   1, GL_FALSE, glm::value_ptr(miniNM));
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(miniMapView));

        glm::mat4 miniMVP = orthogonalProjection * miniMapView * model;
        if (locMVP >= 0) glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(miniMVP));

        renderObjects(programID, miniMapView);
        
        glDisable(GL_SCISSOR_TEST);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    } while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);

    if (gSpatialHash) delete gSpatialHash;
    glDeleteProgram(programID);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}