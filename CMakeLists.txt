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

file(COPY
    "vertexShader.glsl"
    "fragmentShader.glsl"
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

file(COPY
    "Models/Hole_1_Dog.glb"
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Models
)


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

# Common source files shared by both executables
set(COMMON_SOURCES
    "Camera.cpp"
    "Drone.cpp"
    "Figure.cpp"
    "Object.cpp"
    "RenderState.cpp"
    "Shape3D.cpp"
    "shader.cpp"
    "SpatialHash.cpp"
    "LayoutReader.cpp"
    "Shapes/light.cpp"
    "Hole8.cpp"   
    "Hole9.cpp"   
    "Hole10.cpp"
    "Hole13.cpp"
    "Hole4.cpp"
)

# Create single executable from HomwWorkAssignment.cpp
add_executable(HomwWorkAssignment
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

file(GLOB ASSIMP_RUNTIME_DLLS
    "${assimp_BINARY_DIR}/bin/*.dll"
)
if(ASSIMP_RUNTIME_DLLS)
    file(COPY ${ASSIMP_RUNTIME_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(glfw3_FOUND)
    target_link_libraries(HomwWorkAssignment PRIVATE glfw)
endif()

if(GLEW_FOUND)
    target_link_libraries(HomwWorkAssignment PRIVATE GLEW::GLEW)
endif()

# C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET HomwWorkAssignment PROPERTY CXX_STANDARD 17)
endif()

# Set HomwWorkAssignment as the startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HomwWorkAssignment)

# TODO: Add tests and install targets if needed.