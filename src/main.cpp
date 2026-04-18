#include <iostream>
#include <vector>
#include <algorithm> // for std::min_element / std::max_element
#include <FastNoise/FastNoise.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION  // Only define this ONCE across the whole project - lives here in main.cpp
#include "../lib/stb_image_write.h"

#include "../include/worldgen.h"        // ADDED: shared header for WorldGenConfig + function declarations

// ============================================================
// GetBiomeColor - backend logic, stays in main.cpp
// ============================================================
static void GetBiomeColor(float n, int worldType, unsigned char& r, unsigned char& g, unsigned char& b) {
	switch (worldType) {
	case 0: // Desert
		if (n < 0.4f) { r = 180; g = 140; b = 80; } // Flat sands
		else if (n < 0.7f) { r = 210; g = 170; b = 100; } // Dunes
		else { r = 160; g = 110; b = 60; } // Rocky areas
		break;

	case 1: // Forest
		if (n < 0.3f) { r = 30;  g = 100; b = 180; } // Lakes
		else if (n < 0.6f) { r = 34;  g = 139; b = 34; } // Forest floor
		else if (n < 0.8f) { r = 120; g = 100; b = 80; } // Cobblestones
		else { r = 240; g = 240; b = 255; }// Snow caps
		break;

	case 2: // Mountain
		if (n < 0.3f) { r = 50;  g = 80;  b = 160; } // Ocean
		else if (n < 0.5f) { r = 100; g = 90;  b = 70; } // Hills
		else if (n < 0.75f) { r = 120; g = 100; b = 80; } // Stone
		else { r = 240; g = 240; b = 255; } // Snow caps
		break;

	case 3: // Alien - ADDED: was missing entirely before
		if (n < 0.35f) { r = 20;  g = 0;   b = 80; } // Purple sea
		else if (n < 0.6f) { r = 180; g = 0;   b = 200; } // Magenta plains
		else if (n < 0.8f) { r = 0;   g = 220; b = 180; } // Teal highland
		else { r = 255; g = 255; b = 0; } // Yellow peaks
		break;

	default: { r = 128; g = 128; b = 128; } // Fallback grey
		   break;
	}
}

// ============================================================
// GenerateWorld - core backend function
// Called by ui.cpp when the user hits Generate or Regenerate
// ============================================================
bool GenerateWorld(const WorldGenConfig& config,
	std::vector<unsigned char>& outPixels,
	const char* outputPath) {

	std::cout << "Generating " << config.width << "x" << config.height
		<< " | Seed: " << config.seed
		<< " | WorldType: " << config.worldType
		<< " | Octaves: " << config.octaves
		<< " | Frequency: " << config.frequency << std::endl;

	// Build noise graph
	auto simplex = FastNoise::New<FastNoise::Simplex>();
	auto fnSimplex = FastNoise::New<FastNoise::FractalFBm>();
	fnSimplex->SetSource(simplex);
	fnSimplex->SetOctaveCount(config.octaves);

	if (!simplex || !fnSimplex) {
		std::cerr << "Failed to create FastNoise node." << std::endl;
		return false;
	}

	// Generate raw noise into flat array
	std::vector<float> noiseOutput(config.width * config.height);
	fnSimplex->GenUniformGrid2D(
		noiseOutput.data(),
		0, 0,
		config.width, config.height,
		config.frequency, config.frequency,
		config.seed
	);

	// Find min/max for normalization
	float minVal = *std::min_element(noiseOutput.begin(), noiseOutput.end());
	float maxVal = *std::max_element(noiseOutput.begin(), noiseOutput.end());
	float range = (maxVal - minVal <= 0.0f) ? 1.0f : (maxVal - minVal);

	std::cout << "Noise range: [" << minVal << ", " << maxVal << "]" << std::endl;

	// Convert to RGB pixels using biome colors
	// outPixels.resize(config.width * config.height);        
	outPixels.resize(config.width * config.height * 3);       // CHANGED: RGB (3 channels)

	for (int i = 0; i < (int)noiseOutput.size(); i++) {
		float n = (noiseOutput[i] - minVal) / range;
		n = std::max(0.0f, std::min(1.0f, n));                // clamp 0-1

		unsigned char r, g, b;
		GetBiomeColor(n, config.worldType, r, g, b);

		outPixels[i * 3 + 0] = r;                            
		outPixels[i * 3 + 1] = g;
		outPixels[i * 3 + 2] = b;
	}

	// Save PNG
	int result = stbi_write_png(                          
		outputPath,
		config.width, config.height,
		3,
		outPixels.data(),
		config.width * 3
	);

	if (result) {
		std::cout << "Success! Saved: " << outputPath << std::endl;
		return true;
	}
	else {
		std::cerr << "ERROR: Failed to save image." << std::endl;
		return false;
	}
}

// ============================================================
// main() - entry point
// CHANGED: no longer runs standalone generation
// Now just boots the UI by calling RunUI() from ui.cpp
// ============================================================
int main() {
	return RunUI();

	// OLD standalone code - kept for reference
	// std::cout << "WorldGen Starting..." << std::endl;
	// WorldGenConfig config;
	// config.width     = 256;
	// config.height    = 256;
	// config.seed      = 1337;
	// config.frequency = 4.0f;
	// config.octaves   = 5;
	// config.worldType = 1;
	// std::vector<unsigned char> pixels;
	// bool success = GenerateWorld(config, pixels, "heightmap.png");
	// if (success) { std::cout << "Done! heightmap.png saved." << std::endl; }
	// std::cout << "\nPress Enter to exit..." << std::endl;
	// std::cin.get();
	// return 0;
}