# CMakeList.txt : CMake project for HomwWorkAssignment, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

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

# Try to find other libraries (optional - will use system if available)
find_package(glfw3 CONFIG QUIET)
find_package(GLEW CONFIG QUIET)

# Fetch Assimp from GitHub
FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.3.1
)
FetchContent_MakeAvailable(assimp)

# Add source to this project's executable.
add_executable (HomwWorkAssignment "HomwWorkAssignment.cpp" "HomwWorkAssignment.h")

# Include GLM headers for this target
target_include_directories(HomwWorkAssignment PRIVATE ${glm_SOURCE_DIR})

# Link libraries to the target
target_link_libraries(HomwWorkAssignment PRIVATE 
    assimp
)

# Link optional libraries if found
if(glfw3_FOUND)
    target_link_libraries(HomwWorkAssignment PRIVATE glfw)
endif()

if(GLEW_FOUND)
    target_link_libraries(HomwWorkAssignment PRIVATE GLEW::GLEW)
endif()

# Include Assimp headers
target_include_directories(HomwWorkAssignment PRIVATE ${assimp_SOURCE_DIR}/include)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HomwWorkAssignment PROPERTY CXX_STANDARD 17)
endif()

# TODO: Add tests and install targets if needed.
