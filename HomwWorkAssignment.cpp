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

// Global spatial hash for efficient object queries
// Cell size 2.5f creates 8x8 = 64 cells (quadrants with 4 subdivisions each)
SpatialHash* gSpatialHash = nullptr;


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
                gUseGrayscale = !gUseGrayscale;  // Toggle grayscale filter
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




void renderObjects(GLuint programID, glm::mat4 view) {
    // Render skybox (rotation only, no translation)
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(glGetUniformLocation(programID, "uView"), 1, GL_FALSE, glm::value_ptr(skyboxView));
    glDepthMask(GL_FALSE); 
    
    // TODO: Render skybox here
    // <- add skybox render and uncomment
    
    glDepthMask(GL_TRUE);   // Re-enable depth writing so no sky box does no interfere with the other renders
    
    glUniformMatrix4fv(glGetUniformLocation(programID, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    
    // Render other objects
    // TODO: Add your object rendering here
    // cube.render(programID);
    // sphere.render(programID);
    // etc.
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
   
    // Register key callback
    glfwSetKeyCallback(window, keyCallback);
    
	GLint locNM = glGetUniformLocation(programID, "uNormalMatrix");
    GLint locMVP = glGetUniformLocation(programID, "uMVP");
    GLint locView = glGetUniformLocation(programID, "uView");
    GLint locModel = glGetUniformLocation(programID, "uModel");
    GLint locProjection = glGetUniformLocation(programID, "uProjection");
    
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    // Set up perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 orthogonalProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 model = glm::mat4(1.0f);
    
    glUseProgram(programID);
    glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    
    float cameraSpeed = 0.1f;

	// Initialize spatial hash with 2.5f cell size (creates 8x8 grid with 4 subdivisions per quadrant)
    gSpatialHash = new SpatialHash(2.5f);
    
    // TODO: Add your objects here and insert them into the spatial hash
    // Example:
    // Sphere<4> ball(1.0f);
    // Figure figure(&ball, glm::vec3(0.0f, 0.0f, 0.0f));
    // int objectID = 0;
    // gSpatialHash->insert(objectID++, ball.getPosition());
    
	// Start object initialisation here


    
    do {
        // Handle camera movement using key listener
        if (keyState.W)
            cameraPos += cameraSpeed * glm::normalize(cameraTarget - cameraPos);
        if (keyState.S)
            cameraPos -= cameraSpeed * glm::normalize(cameraTarget - cameraPos);
        if (keyState.A)
            cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraTarget - cameraPos, upVector));
        if (keyState.D)
            cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraTarget - cameraPos, upVector));
        
        // Update view matrix with new camera position
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Set grayscale uniform
        GLint grayscaleLoc = glGetUniformLocation(programID, "useGrayscale");
        if (grayscaleLoc != -1) glUniform1i(grayscaleLoc, gUseGrayscale ? 1 : 0);
        
        // Render mini-map in bottom right corner using scissor test
        glEnable(GL_SCISSOR_TEST);
        glScissor(750, 0, 250, 250);  // Bottom right corner: 250x250 pixels
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // Set orthogonal projection and top-down view for mini-map
        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(orthogonalProjection));
        glm::mat4 miniMapView = glm::lookAt(
            glm::vec3(0.0f, 10.0f, 0.0f),  // Look from above
            glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
            glm::vec3(0.0f, 0.0f, -1.0f)   // Up vector pointing towards negative Z
        );
        renderObjects(programID, miniMapView);
        
        glDisable(GL_SCISSOR_TEST);
        
        // Example: Query spatial hash for nearby objects at camera position
        // This demonstrates hierarchical spatial partitioning
        if (gSpatialHash) {
            std::vector<int> nearbyObjects = gSpatialHash->queryNearby(cameraPos);
            // nearbyObjects contains IDs of all objects in nearby cells (3x3x3 cube)
            // Use this for collision detection, culling, or other spatial queries
        }
        
        // Render main view
        glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
        view = glm::lookAt(cameraPos, cameraTarget, upVector);
        renderObjects(programID, view);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    } while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
    
    // Clean up spatial hash
    if (gSpatialHash) {
        delete gSpatialHash;
        gSpatialHash = nullptr;
    }
    
    glDeleteProgram(programID);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
