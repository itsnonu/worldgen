cmake_minimum_required(VERSION 3.15)
project (WorldGen)

# Use C++17 (FastNoise2 requires at least C++17)
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add FastNoise2 as a subdirectory
# Builds the FastNoise2 library first, making it available for linking

add_subdirectory(lib/FastNoise2)

# Locate FastNoise2 headers and include them in the project
include_directories(${CMAKE_SOURCE_DIR}lib/FastNoise2/include)

# Create executable from main.cpp and link it with the FastNoise2 library
add_executable(WorldGen src/main.cpp)

# Link FastNoise2 Library to the executable so that the functions and features of FastNoise2 can be used in WorldGen
target_link_libraries(WorldGen FastNoise2)