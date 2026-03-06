#include <iostream>
#include <vector>
#include <FastNoise/FastNoise.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
	std::cout << "WorldGen Starting..." << std::endl;

	// Grid dimensions 
	const int width = 256; // 256 pixels wide
	const int height = 256; // 256 pixels tall

	std::cout << "Generating " << width << "x" << height << " heightmap..." << std::endl;

	// Create a FastNoise node - Using Simplex noise type
	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();

	// Check if the node was created successfully
	if (!fnSimplex) {
		std::cerr << "Failed to create FastNoise node." << std::endl;
		return 1;
	}

	// Create a 2D vector to store noise values
	std::vector<std::vector<float>> heightmap(height, std::vector<float>(width));

	std::cout << "Generating noise..." << std::endl;

	std::cout << "Generating noise..." << std::endl;

	// Generate noise using the correct API
	std::vector<float> noiseOutput(width * height);

	// GenUniformGrid2D parameters: output, xStart, yStart, xSize, ySize, frequency
	fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0.0f, 0.0f, width, height, 0.02f, 0.02f, 1337);

	// Copy the 1D noise output into our 2D heightmap array
	std::cout << "Organizing heightmap data..." << std::endl;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			heightmap[y][x] = noiseOutput[y * width + x];
		}

		// Progress indicator
		if (y % 50 == 0) {
			std::cout << "Progress: " << y << "/" << height << std::endl;
		}
	}
	std::cout << "Noise generation complete!" << std::endl;

	// Debug section - Check noise values
	std::cout << "\n=== DEBUG - Checking noise values ===" << std::endl;
	std::cout << "Sample values from heightmap:" << std::endl;
	std::cout << "heightmap[0][0] = " << heightmap[0][0] << std::endl;
	std::cout << "heightmap[50][50] = " << heightmap[50][50] << std::endl;
	std::cout << "heightmap[100][100] = " << heightmap[100][100] << std::endl;
	std::cout << "heightmap[150][150] = " << heightmap[150][150] << std::endl;
	std::cout << "heightmap[200][200] = " << heightmap[200][200] << std::endl;

	// Find min and max values for normalization
	float minVal = heightmap[0][0];
	float maxVal = heightmap[0][0];
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (heightmap[y][x] < minVal) minVal = heightmap[y][x];
			if (heightmap[y][x] > maxVal) maxVal = heightmap[y][x];
		}
	}
	std::cout << "Min noise value: " << minVal << std::endl;
	std::cout << "Max noise value: " << maxVal << std::endl;
	std::cout << "Range: " << (maxVal - minVal) << std::endl;
	std::cout << "=================================\n" << std::endl;

	// Convert heightmap to image pixels
	std::cout << "Converting to image..." << std::endl;

	std::vector<unsigned char> pixels(width * height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float noise = heightmap[y][x];

			// Normalize to 0-1 range based on actual min/max
			float normalized = (noise - minVal) / (maxVal - minVal);

			// Convert to 0-255 pixel value
			unsigned char pixel = (unsigned char)(normalized * 255.0f);

			// Store in 1D pixel array
			pixels[y * width + x] = pixel;
		}
	}

	std::cout << "Saving image..." << std::endl;

	// Save as PNG file
	int result = stbi_write_png("heightmap.png", width, height, 1, pixels.data(), width);

	if (result) {
		std::cout << "Success! Heightmap saved as 'heightmap.png'" << std::endl;
		std::cout << "Location: build/Debug/heightmap.png" << std::endl;
	}
	else {
		std::cout << "ERROR: Failed to save image!" << std::endl;
	}

	std::cout << "\nPress Enter to exit..." << std::endl;
	std::cin.get();

	return 0;
}