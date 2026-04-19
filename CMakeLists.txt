cmake_minimum_required(VERSION 3.15)
project(WorldGen)

# Use C++17 (FastNoise2 requires at least C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add FastNoise2 as a subdirectory
add_subdirectory(lib/FastNoise2)

# Disable GLFW extras we don't need
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)

# Define ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    # Backends
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/lib/FastNoise2/include)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/include)          # ADDED: for worldgen.h
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(${CMAKE_SOURCE_DIR}/lib/glfw/include)

# added src/ui.cpp so both files are compiled into the same executable
add_executable(WorldGen
    src/main.cpp
    src/ui.cpp
    ${IMGUI_SOURCES}
)

# Link libraries
target_link_libraries(WorldGen FastNoise2 glfw ${OPENGL_LIBRARIES})