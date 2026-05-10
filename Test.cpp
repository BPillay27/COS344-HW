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
unsigned int rotationSpeed=0;
vector<Object*> scene;
// Global directional light
directionalLight* gDirectionalLight = nullptr;
// Global point light
pointLight* gPointLight = nullptr;
// Debug toggles (removed debugAlpha/invertAlpha/alphaMode)

// Global pointers to allow runtime modification of shapes' tessellation
Sphere<4>* gBallPtr = nullptr;
Cylinder<4>* gCylinderPtr = nullptr; // (kept for compatibility in other shapes)
// (no separate visualiser sphere; light is represented by lighting only)

// Pending tessellation changes applied by the main loop to avoid heavy work in callbacks
int gPendingBallStacksDelta = 0;
int gPendingBallSlicesDelta = 0;
// Global CPU light state (transformed with scene rotations)
glm::vec4 gLightPos = glm::vec4(0.0f, 0.6f, 0.0f, 1.0f);
glm::vec4 gLightCol = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
float gLightIntensity = 15.0f;  // Increased from 5.0 for better visibility
float gLightRange = 5.0f;
// Light follow flag: when true, light follows sphere center; when false, light moves independently
bool gLightFollowBall = true;

// Colour palette and indices for floor, ball, and light
struct Colour {
    int r,g,b; 
    float a;
 };
    static std::vector<Colour> gPalette = {
        {255,0,0,1.0f},   // Red
        {0,255,0,1.0f},   // Green
        {0,0,255,1.0f},   // Blue
        {255,255,255,1.0f}, // White
        {0,0,0,1.0f},     // Black
        {255,255,0,1.0f}, // Yellow
        {255,0,255,1.0f}, // Magenta
        {0,255,255,1.0f}, // Cyan
        {255,128,0,1.0f}, // Orange
        {125,125,124,1.0f}, //The start color of plane
        {60,55,25,1.0f}
    };
static int gFloorColIdx = 9;
static int gBallColIdx = 3; // default white-ish glass
static int gLightColIdx = 3;

static float ballApha=0.3f;

void key_listener(GLFWwindow* window, int key, int scancode, int action, int mods){

    if(key==GLFW_KEY_ENTER && action==GLFW_PRESS){
        wireframeMode=!wireframeMode;
        if(wireframeMode){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    // Dynamic geometry controls
    if(action==GLFW_PRESS){
        // Sphere tessellation: '[' ']' to decrease/increase stacks & slices
        if(key==GLFW_KEY_LEFT_BRACKET){
            // Defer heavy tessellation update to main loop
            gPendingBallStacksDelta -= 1;
            gPendingBallSlicesDelta -= 1;
        } else if(key==GLFW_KEY_RIGHT_BRACKET){
            gPendingBallStacksDelta += 1;
            gPendingBallSlicesDelta += 1;
        }

        // Cylinder tessellation: ',' '.' to decrease/increase resolution
        if(key==GLFW_KEY_COMMA){
            // if SHIFT is held, move light in local -Z
            if (mods & GLFW_MOD_SHIFT) {
                gLightPos[2] -= 0.05f; gLightFollowBall = false;
            } else if(gCylinderPtr){
                int r = gCylinderPtr->getResolution();
                if(r>1) gCylinderPtr->setResolution(r-1);
            }
        } else if(key==GLFW_KEY_PERIOD){
            // if SHIFT is held, move light in local +Z
            if (mods & GLFW_MOD_SHIFT) {
                gLightPos[2] += 0.05f; gLightFollowBall = false;
            } else if(gCylinderPtr){
                int r = gCylinderPtr->getResolution();
                if(r<1024) gCylinderPtr->setResolution(r+1);
            }
        }

        // Shader toggles and alpha control
        if(key==GLFW_KEY_B){ // toggle colour texture
            gUseColor = !gUseColor;
        }
        if(key==GLFW_KEY_N){ // toggle displacement
            gUseDisplacement = !gUseDisplacement;
        }
        if(key==GLFW_KEY_M){ // toggle alpha map
            gUseAlphaMap = !gUseAlphaMap;
        }

        // Colour cycling keys (choose two per object: prev/next)
        // Floor: J (prev), K (next)
        if (key == GLFW_KEY_J) {
            gFloorColIdx = (gFloorColIdx - 1 + (int)gPalette.size()) % (int)gPalette.size();
            if (gCylinderPtr) {
                Colour c = gPalette[gFloorColIdx];
                gCylinderPtr->setColour(c.r, c.g, c.b, c.a);
            }
        }
        if (key == GLFW_KEY_K) {
            gFloorColIdx = (gFloorColIdx + 1) % (int)gPalette.size();
            if (gCylinderPtr) {
                Colour c = gPalette[gFloorColIdx];
                gCylinderPtr->setColour(c.r, c.g, c.b, c.a);
            }
        }

        // Ball colours: O (prev), P (next)
        if (key == GLFW_KEY_O) {
            gBallColIdx = (gBallColIdx - 1 + (int)gPalette.size()) % (int)gPalette.size();
            if (gBallPtr) {
                Colour c = gPalette[gBallColIdx];
                gBallPtr->setColour(c.r, c.g, c.b, ballApha);
            }
        }
        if (key == GLFW_KEY_P) {
            gBallColIdx = (gBallColIdx + 1) % (int)gPalette.size();
            if (gBallPtr) {
                Colour c = gPalette[gBallColIdx];
                gBallPtr->setColour(c.r, c.g, c.b, ballApha);
            }
        }

        // Light colours: U (prev), I (next)
        if (key == GLFW_KEY_U) {
            gLightColIdx = (gLightColIdx - 1 + (int)gPalette.size()) % (int)gPalette.size();
                Colour c = gPalette[gLightColIdx];
            gLightCol[0] = c.r / 255.0f; gLightCol[1] = c.g / 255.0f; gLightCol[2] = c.b / 255.0f; gLightCol[3] = c.a;
        }
        if (key == GLFW_KEY_I) {
            gLightColIdx = (gLightColIdx + 1) % (int)gPalette.size();
            Colour c = gPalette[gLightColIdx];
            gLightCol[0] = c.r / 255.0f; gLightCol[1] = c.g / 255.0f; gLightCol[2] = c.b / 255.0f; gLightCol[3] = c.a;
        }

        // Alpha value control: + and - keys (both keypad and main keys)
        if(key==GLFW_KEY_KP_ADD || (key==GLFW_KEY_EQUAL && mods & GLFW_MOD_SHIFT)){
            ballApha += 0.1f;
            if(ballApha>1.0f) ballApha = 1.0f;
            if (gBallPtr) {
                Colour c = gPalette[gBallColIdx];
                gBallPtr->setColour(c.r, c.g, c.b, ballApha);
            }
           
        }
        if(key==GLFW_KEY_KP_SUBTRACT || (key==GLFW_KEY_MINUS && mods & GLFW_MOD_SHIFT)){
            ballApha -= 0.1f;
            if(ballApha<0.0f) ballApha = 0.0f;
            if (gBallPtr) {
                Colour c = gPalette[gBallColIdx];
                gBallPtr->setColour(c.r, c.g, c.b, ballApha);
            }
        }
        // Rotations: W/S -> rotate X, A/D -> rotate Y, E/Q -> rotate Z (15 degrees)
        if(key==GLFW_KEY_W){
            for(Object* obj : scene) obj->rotateX(15);
            // keep CPU light at sphere center after rotating the scene (only if following)
            if (gBallPtr && gLightFollowBall) gLightPos = gBallPtr->center;
        }
        if(key==GLFW_KEY_S){
            for(Object* obj : scene) obj->rotateX(-15);
            if (gBallPtr && gLightFollowBall) gLightPos = gBallPtr->center;
        }
        if(key==GLFW_KEY_A){
            for(Object* obj : scene) obj->rotateY(15);
            if (gBallPtr && gLightFollowBall) gLightPos = gBallPtr->center;
        }
        if(key==GLFW_KEY_D){
            for(Object* obj : scene) obj->rotateY(-15);
            if (gBallPtr && gLightFollowBall) gLightPos = gBallPtr->center;
        }
        if(key==GLFW_KEY_E){
            for(Object* obj : scene) obj->rotateZ(15);
            if (gBallPtr && gLightFollowBall) gLightPos = gBallPtr->center;
        }
        if(key==GLFW_KEY_Q){
            for(Object* obj : scene) obj->rotateZ(-15);
            if (gBallPtr && gLightFollowBall) gLightPos = gBallPtr->center;
        }
        
        
        if(key==GLFW_KEY_UP){ gLightPos[1] += 0.05f; gLightFollowBall = false; }
        if(key==GLFW_KEY_DOWN){ gLightPos[1] -= 0.05f; gLightFollowBall = false; }
        if(key==GLFW_KEY_LEFT){ gLightPos[0] -= 0.05f; gLightFollowBall = false; }
        if(key==GLFW_KEY_RIGHT){ gLightPos[0] += 0.05f; gLightFollowBall = false; }
        // Use SHIFT+',' and SHIFT+'.' (COMMA/PERIOD) to move light in Z if requested
        // Reset: Space key resets light follow and repositions light to ball centre
        if(key==GLFW_KEY_SPACE){ 
            // reset follow behaviour and position
            gLightFollowBall = true;
            if (gBallPtr) gLightPos = gBallPtr->center;
            // reset colours to defaults from palette
            gFloorColIdx = 9; //Grey
            gBallColIdx = 10; // white
            gLightColIdx = 3; // white
            Colour cf = gPalette[gFloorColIdx];
            Colour cb = gPalette[gBallColIdx];
            Colour cl = gPalette[gLightColIdx];
            if (gCylinderPtr) gCylinderPtr->setColour(cf.r, cf.g, cf.b, cf.a);
            // reset ball alpha and colour
            gAlphaValue = 0.3f;
            ballApha = gAlphaValue;
            if (gBallPtr) gBallPtr->setColour(cb.r, cb.g, cb.b, ballApha);
            gLightCol[0] = cl.r / 255.0f; gLightCol[1] = cl.g / 255.0f; gLightCol[2] = cl.b / 255.0f; gLightCol[3] = cl.a;
            // reset alpha of ball
            gAlphaValue = 0.3f;
        }
        
        
    }
}

// Note: GPU shader now handles all lighting (directional, point, ambient)
// No longer need CPU-based per-vertex color computation

const char *getError()
{
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW()
{
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        throw getError();
    }
}

inline void startUpGLEW()
{
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp()
{
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);               // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
    GLFWwindow *window;                                            // (In the accompanying source code, this variable is global for simplicity)
    window = glfwCreateWindow(1000, 1000, "Experiment", NULL, NULL);
    if (window == NULL)
    {
        cout << getError() << endl;
        glfwTerminate();
        throw "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n";
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    startUpGLEW();
    return window;
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

    //Add code here

    GLuint programID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    if (programID == 0) { std::cerr << "Failed to load shaders" << std::endl; glfwTerminate(); return 1; }
    glEnable(GL_DEPTH_TEST);
    // Enable alpha blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(programID);
    // Use normal filled rendering and enable face culling
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    
    // Initialize directional light
    gDirectionalLight = new directionalLight(
        glm::vec4(0.5f, 1.0f, 0.5f, 0.0f),  // Direction (normalized)
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),  // Color (white)
        1.0f                                 // Intensity
    );
    std::cout << "Directional light created successfully." << std::endl;
    
    // Initialize point light - will follow gLightPos (keyboard controlled and ball-tracking)
    gPointLight = new pointLight(
        gLightPos,              // Position (will be updated each frame to follow gLightPos)
        gLightCol,              // Color (controlled by U/I keys)
        gLightIntensity,        // Intensity
        gLightRange             // Range
    );
    std::cout << "Point light created successfully." << std::endl;

    // Load textures (colour, alpha, displacement)
    auto loadTextureFromFile = [&](const char* path, bool flipY=true)->GLuint{
        if (flipY) stbi_set_flip_vertically_on_load(1);
        int imgWidth, imgHeight, nrChannels;
        unsigned char* data = stbi_load(path, &imgWidth, &imgHeight, &nrChannels, 0);
        if (!data) {
            std::cerr << "Failed to load texture: " << path;
            #if defined(STBI_FAILURE_REASON)
            std::cerr << " (stbi failure: " << stbi_failure_reason() << ")";
            #else
            std::cerr << " (stbi failure: unknown)";
            #endif
            std::cerr << std::endl;
            return 0u;
        }
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, format, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return texID;
    };

    GLuint colorTex = loadTextureFromFile("Textures/colour/colour.png");
    GLuint displacementTex = loadTextureFromFile("Textures/displacement/displacement.png");
    GLuint alphaTex = loadTextureFromFile("Textures/alpha/alpha.png");
    if (colorTex == 0u) {
        unsigned char white[4] = {255,255,255,255};
        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    }
    if (displacementTex == 0u) {
        unsigned char mid = 128;
        glGenTextures(1, &displacementTex);
        glBindTexture(GL_TEXTURE_2D, displacementTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &mid);
    }
    if (alphaTex == 0u) {
        unsigned char white = 255;
        glGenTextures(1, &alphaTex);
        glBindTexture(GL_TEXTURE_2D, alphaTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &white);
    }

    // Ensure alpha and displacement maps do not repeat at UV seams
    if (displacementTex != 0u) {
        glBindTexture(GL_TEXTURE_2D, displacementTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    if (alphaTex != 0u) {
        glBindTexture(GL_TEXTURE_2D, alphaTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Set sampler uniforms to texture units
    glUseProgram(programID);
    GLint locColor = glGetUniformLocation(programID, "colorMap");
    if (locColor != -1) glUniform1i(locColor, 0); // GL_TEXTURE0
    GLint locAlpha = glGetUniformLocation(programID, "alphaMap");
    if (locAlpha != -1) glUniform1i(locAlpha, 1); // GL_TEXTURE1
    GLint locDisp = glGetUniformLocation(programID, "displacementMap");
    if (locDisp != -1) glUniform1i(locDisp, 2); // GL_TEXTURE2

    auto setAttribcolour = [](float r, float g, float b, float a) {
        glDisableVertexAttribArray(1);
        glVertexAttrib4f(1, r, g, b, a);
    };
    glClearColor(0.1f,0.1f,0.1f,1.0f);


    //Creation of the 3D objcts starts here:
    {

        float planeHalf = 0.8f;
        float alignedx = 0.0f;
        float alignedy = 0.0f;
        float centreZ= 0.0f;

        glm::vec4 ccenter = glm::vec4(alignedx, alignedy, centreZ, 1.0f);
        Cylinder<4>* floorCyl = new Cylinder<4>(ccenter, planeHalf, 0.01f, 64, 1);
        floorCyl->setColour(154, 154, 154, 1.0f);

        floorCyl->createGLBuffers();
        gCylinderPtr = floorCyl;

        float radius = 0.25f;
        Sphere<4>* ball = new Sphere<4>({0.0f, alignedy + radius + 0.01f, 0.0f, 1.0f}, radius, 6, 8);
        gBallPtr = ball;

        Figure planed=Figure();
        planed.addShape(floorCyl);

    ball->setColour(60,55,25,0.3f);
    // creating GL buffers for ball
    ball->createGLBuffers();
    // Place the CPU light at the sphere center
    gLightPos = ball->center;
    Shape3D golfBall(ball);


    
    scene.push_back(&golfBall);
    scene.push_back(&planed);
    
   
    


    glfwSetKeyCallback(window,key_listener);

    do{ 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        
        // Set polygon mode based on wireframe toggle
        if(wireframeMode){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // Set directional light uniforms
        if (gDirectionalLight) {
            GLint locDir = glGetUniformLocation(programID, "uLightDir");
            GLint locCol = glGetUniformLocation(programID, "uLightColor");
            GLint locInt = glGetUniformLocation(programID, "uLightIntensity");
            GLint locNM = glGetUniformLocation(programID, "uNormalMatrix");
            
            if (locDir != -1) {
                glm::vec3 lightDir = glm::normalize(glm::vec3(gDirectionalLight->getDirection()));
                glUniform3f(locDir, lightDir.x, lightDir.y, lightDir.z);
            }
            if (locCol != -1) {
                glm::vec3 lightCol = glm::vec3(gDirectionalLight->getColor());
                glUniform3f(locCol, lightCol.x, lightCol.y, lightCol.z);
            }
            if (locInt != -1) {
                glUniform1f(locInt, gDirectionalLight->getIntensity());
            }
            if (locNM != -1) {
                // Use identity normal matrix for now (no model transformation)
                glm::mat3 normalMatrix = glm::mat3(1.0f);
                glUniformMatrix3fv(locNM, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
        }
        
        // Update point light position to follow gLightPos and set uniforms
        if (gPointLight) {
            // Sync point light position and color with global light state
            gPointLight->setPosition(gLightPos);
            gPointLight->setColor(gLightCol);
            
            GLint locPos = glGetUniformLocation(programID, "uPointLightPos");
            GLint locCol = glGetUniformLocation(programID, "uPointLightColor");
            GLint locInt = glGetUniformLocation(programID, "uPointLightIntensity");
            GLint locRange = glGetUniformLocation(programID, "uPointLightRange");
            
            if (locPos != -1) {
                glm::vec3 lightPos = glm::vec3(gPointLight->getPosition());
                glUniform3f(locPos, lightPos.x, lightPos.y, lightPos.z);
            }
            if (locCol != -1) {
                glm::vec3 lightCol = glm::vec3(gPointLight->getColor());
                glUniform3f(locCol, lightCol.x, lightCol.y, lightCol.z);
            }
            if (locInt != -1) {
                glUniform1f(locInt, gPointLight->getIntensity());
            }
            if (locRange != -1) {
                glUniform1f(locRange, gPointLight->getRange());
            }
        }

        if (gBallPtr && (gPendingBallStacksDelta != 0 || gPendingBallSlicesDelta != 0)) {
            int newStacks = gBallPtr->stacks + gPendingBallStacksDelta;
            int newSlices = gBallPtr->slices + gPendingBallSlicesDelta;
            if (newStacks < 3) newStacks = 3;
            if (newSlices < 3) newSlices = 3;
            if (newStacks > 256) newStacks = 256;
            if (newSlices > 256) newSlices = 256;
            gBallPtr->stacks = newStacks;
            gBallPtr->slices = newSlices;
            gBallPtr->createGLBuffers();
            if (gLightFollowBall && gBallPtr) {
                gLightPos = gBallPtr->center;
            }
            gPendingBallStacksDelta = 0;
            gPendingBallSlicesDelta = 0;
        }


        for (Object* s : scene) {
            Shape3D* sd = dynamic_cast<Shape3D*>(s);
            bool isBall = (sd && sd->getShape() == gBallPtr);
            if (!isBall) {
                GLint loc;
                loc = glGetUniformLocation(programID, "useColor"); if (loc != -1) glUniform1i(loc, 0);
                loc = glGetUniformLocation(programID, "useAlphaMap"); if (loc != -1) glUniform1i(loc, 0);
                loc = glGetUniformLocation(programID, "useDisplacement"); if (loc != -1) glUniform1i(loc, 0);
                glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
                glDisable(GL_CULL_FACE);  // Disable backface culling for floor (complex winding)
                s->draw();
                glEnable(GL_CULL_FACE);   // Re-enable for ball
            }
        }

        for (Object* s : scene) {
            Shape3D* sd = dynamic_cast<Shape3D*>(s);
            if (sd && sd->getShape() == gBallPtr) {
                glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, colorTex);
                glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, alphaTex);
                glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, displacementTex);

                GLint loc;
                loc = glGetUniformLocation(programID, "useColor"); if (loc != -1) glUniform1i(loc, gUseColor ? 1 : 0);
                loc = glGetUniformLocation(programID, "useAlphaMap"); if (loc != -1) glUniform1i(loc, gUseAlphaMap ? 1 : 0);
                loc = glGetUniformLocation(programID, "useDisplacement"); if (loc != -1) glUniform1i(loc, gUseDisplacement ? 1 : 0);
                loc = glGetUniformLocation(programID, "displacementScale"); if (loc != -1) glUniform1f(loc, gDisplacementScale);

                loc = glGetUniformLocation(programID, "uBaseColor");
                if (loc != -1) {
                    float* col = sd->getShape()->getColour();
                    float baseAlpha = col[3];

                    if (gUseAlphaMap) baseAlpha *= gAlphaValue;
                    float baseCol[4] = { col[0], col[1], col[2], baseAlpha };
                    glUniform4fv(loc, 1, baseCol);
                    delete[] col;
                }
                glDepthMask(GL_FALSE);
                s->draw();
                glDepthMask(GL_TRUE);
            }
        }
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window));

    delete floorCyl ;
    delete ball;
    }

    // Clean up directional light
    if (gDirectionalLight) {
        delete gDirectionalLight;
        gDirectionalLight = nullptr;
    }
    
    // Clean up point light
    if (gPointLight) {
        delete gPointLight;
        gPointLight = nullptr;
    }

    glDeleteProgram(programID);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
