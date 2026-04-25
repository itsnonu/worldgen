# WorldGen
A C++ middleware tool for generating dynamic and random game worlds via procedural algorithms.

## Tech Stack
- **Language:** C++17
- **Build System:** CMake
- **Noise Generation:** FastNoise2
- **GUI Framework:** Dear ImGui
- **Visualization:** stb_image_write & stb_image

## Folder Structure
- **src:** .cpp Logic Location
- **include:** .h Header File Location
- **lib:** Third-Party Libraries
- **CMakeLists:** Instructions on how to compile the project

# Prerequisites
- CMake 3.15+
- Visual Studio with the Desktop development with C++ workload, or MinGW-w64
- OpenGL

## Getting Started
1. Clone the repo:
- https://github.com/itsnonu/worldgen.git
2. Open CMake GUI
3. Set your Source and Build paths:
- Locate the repo
4. Configure:
- Click Configure, choose MinGW Makefiles for MinGw or MSVC if Visual Studio
- Click Finish and wait for it to complete
5. Generate:
- Click Generate, if there are no errors, there should be a "Generating done" output
6. Build
- Click Open Project to open it in Visual Studio
7. Run
- Set the "WorldGen.exe" as "set as default startup"
