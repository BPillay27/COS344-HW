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
#include "Drone.h"
#include "Hole8.h"
#include "Hole9.h"
#include "Hole10.h"
#include "Camera.h"

#include "Drone.h"
#include "Hole8.h"
#include "Camera.h"

// Global variables
SpatialHash* gSpatialHash = nullptr;
Figure scene = Figure();
Drone* gDrone = nullptr;



Drone* gDrone = nullptr;

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
    if (!glfwInit()) throw getError();
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
    GLFWwindow *window = glfwCreateWindow(1000, 1000, "Experiment", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw "Failed to open GLFW window.\n";
    }
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

    GLuint programID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
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
    
    float cameraSpeed = 0.0023f;
    gSpatialHash = new SpatialHash(2.5f);
    
    // Drone init
    gDrone = new Drone();
    gDrone->createGLBuffers();

    // Build Hole geometry and register it in the global scene.
    buildHole8(scene, glm::vec3(0.0f, 0.0f, 0.0f));
    buildHole9(scene, glm::vec3(20.0f, 0.0f, -10.0f));
    buildHole10(scene, glm::vec3(28.0f, 0.0f, 25.0f)); // Placed to the right of Hole 9
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
    Square<4> start = Square(startTL, startTR, startBR, startBL);

    // Back Face (Farther rectangle to viewer: +0.025f)
    glm::vec4 startTL2(halfWidth_1-1.0f,  halfHeight_1, centreZ_1 + difference_1 , 1.0f);
    glm::vec4 startTR2( halfWidth_1,  halfHeight_1, centreZ_1 + difference_1 , 1.0f);
    glm::vec4 startBL2(halfWidth_1-1.0f, halfHeight_1-0.24f, centreZ_1 + difference_1 , 1.0f);
    glm::vec4 startBR2( halfWidth_1, halfHeight_1-0.24f, centreZ_1 + difference_1 , 1.0f);
    Square<4> start2 = Square(startTL2, startTR2, startBR2, startBL2);

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
    const float holeHeight_1 = turfHeight_1 + holeExtra;
    const float holeRadius_1 = 0.06f; // reduced radius
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

    Imported<4>* dogModel = new Imported<4>("Hole_1_Dog.glb");
    Shape3D* dogShape = new Shape3D(dogModel);
    dogShape->zoom(-80);
    float dogX = 0.5f;                               // Matches cylinderCenter_1 X
    float dogY = turfTopY_1 + cylinderRadius_1;      // Cylinder Center Y + Radius = Top of cylinder
    float dogZ = turfMidZ_1 + cylinderBackOffsetZ_1;

    dogShape->move(dogX, dogY, dogZ);
    Figure* dogFigure = new Figure();
    dogFigure->addShape3D(dogShape);

    scene.addObject(dogFigure);
    scene.addObject(holeFigure_1); // Add the hole figure to the scene so it renders as a black hole in the turf
    scene.addObject(turf_1);
    scene.addObject(walls_1);

    
    int prismID = scene.getNumShapes() - 1;
    glm::vec3 hashPosition(0.0f, 0.0f, centreZ_1 + 0.0125f);
    if (gSpatialHash != nullptr) gSpatialHash->insert(prismID, hashPosition);

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