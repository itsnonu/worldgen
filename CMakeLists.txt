cmake_minimum_required(VERSION 3.15)
project (WorldGen)

# Use C++17 (FastNoise2 requires at least C++17)
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add FastNoise2 as a subdirectory
# Builds the FastNoise2 library first, making it available for linking

add_subdirectory(lib/FastNoise2)
# Do not include build examples
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)


# Define imgui source file
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    # Backend
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Find Graphics OpenGL
find_package(OpenGL REQUIRED)

# Locate FastNoise2 headers and include them in the project
include_directories(${CMAKE_SOURCE_DIR}/lib/FastNoise2/include)
include_directories(${CMAKE_SOURCE_DIR}/lib)
# dearimgui location
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
# OpenGL
include_directories(${CMAKE_SOURCE_DIR}/lib/glfw/include)

# Create executable from main.cpp and link it with the FastNoise2 library
add_executable(WorldGen src/main.cpp ${IMGUI_SOURCES})

# Link FastNoise2 Library to the executable so that the functions and features of FastNoise2 can be used in WorldGen
# Link OpenGl for graphics
target_link_libraries(WorldGen FastNoise2 glfw ${OPENGL_LIBRARIES})