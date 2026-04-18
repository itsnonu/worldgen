#pragma once
#include <vector>

// ============================================================
// WorldGenConfig
// Shared struct between main.cpp (backend) and ui.cpp (frontend)
// UI fills this out and passes it to GenerateWorld()
// ============================================================
struct WorldGenConfig {
	int width       = 256;   // Small=256, Medium=512, Large=1024
	int height      = 256;
	int seed        = 1337;
	float frequency = 4.0f;
	int octaves     = 5;
	int worldType   = 0;     // 0=Desert, 1=Forest, 2=Mountain, 3=Alien
};

// Declared here so ui.cpp knows GenerateWorld() exists in main.cpp
bool GenerateWorld(const WorldGenConfig& config,
                   std::vector<unsigned char>& outPixels,
                   const char* outputPath = "heightmap.png");

// Declared here so main.cpp knows RunUI() exists in ui.cpp
int RunUI();
