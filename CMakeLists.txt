# CMakeList.txt : CMake project for HomwWorkAssignment, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Use vcpkg for dependency management
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HomwWorkAssignment")

# Include FetchContent for downloading dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# Fetch GLM from GitHub
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
endif()

# Try to find graphics libraries from vcpkg
find_package(glfw3 CONFIG QUIET)
find_package(GLEW QUIET)

# Fetch Assimp from GitHub
FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.3.1
)
FetchContent_MakeAvailable(assimp)

# Add source to this project's executable.
# NOTE: Template implementation files (Shape.cpp, transformation.cpp and all Shape subclass .cpp files)
# are included in their corresponding .h headers and should NOT be listed here to avoid duplicate definitions
add_executable (HomwWorkAssignment
    "Test.cpp"
    "Figure.cpp"
    "Object.cpp"
    "RenderState.cpp"
    "Shape3D.cpp"
    "Drone.cpp"
    "Camera.cpp"
    "shader.cpp"
    "SpatialHash.cpp"
    "LayoutReader.cpp"
    "Shapes/light.cpp"
)

# Create first executable from Test.cpp
add_executable(HomwWorkAssignment 
    "Test.cpp"
    ${COMMON_SOURCES}
)

# Create second executable from HomwWorkAssignment.cpp
add_executable(TestProgram
    "HomwWorkAssignment.cpp"
    ${COMMON_SOURCES}
)

# Configure first executable (HomwWorkAssignment from Test.cpp)
target_include_directories(HomwWorkAssignment PRIVATE ${glm_SOURCE_DIR})
target_include_directories(HomwWorkAssignment PRIVATE ${assimp_SOURCE_DIR}/include)

if(GLEW_FOUND)
    target_include_directories(HomwWorkAssignment PRIVATE ${GLEW_INCLUDE_DIRS})
endif()

if(glfw3_FOUND)
    target_include_directories(HomwWorkAssignment PRIVATE ${GLFW3_INCLUDE_DIR})
endif()

target_link_libraries(HomwWorkAssignment PRIVATE assimp)

if(glfw3_FOUND)
    target_link_libraries(HomwWorkAssignment PRIVATE glfw)
endif()

if(GLEW_FOUND)
    target_link_libraries(HomwWorkAssignment PRIVATE GLEW::GLEW)
endif()

# Configure second executable (TestProgram from HomwWorkAssignment.cpp)
target_include_directories(TestProgram PRIVATE ${glm_SOURCE_DIR})
target_include_directories(TestProgram PRIVATE ${assimp_SOURCE_DIR}/include)

if(GLEW_FOUND)
    target_include_directories(TestProgram PRIVATE ${GLEW_INCLUDE_DIRS})
endif()

if(glfw3_FOUND)
    target_include_directories(TestProgram PRIVATE ${GLFW3_INCLUDE_DIR})
endif()

target_link_libraries(TestProgram PRIVATE assimp)

if(glfw3_FOUND)
    target_link_libraries(TestProgram PRIVATE glfw)
endif()

if(GLEW_FOUND)
    target_link_libraries(TestProgram PRIVATE GLEW::GLEW)
endif()

# C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HomwWorkAssignment PROPERTY CXX_STANDARD 17)
  set_property(TARGET TestProgram PROPERTY CXX_STANDARD 17)
endif()

# Set HomwWorkAssignment as the startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HomwWorkAssignment)

# TODO: Add tests and install targets if needed.
