#include <iostream>
#include <vector>
#include <FastNoise/FastNoise.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"

int main() {
	std::cout << "WorldGen Starting..." << std::endl;

	// Grid dimensions 
	const int width = 256; // 256 pixels wide
	const int height = 256; // 256 pixels tall

	std::cout << "Generating " << width << "x" << height << " heightmap..." << std::endl;

	// Create a FastNoise node - Using Simplex noise type
	auto simplex = FastNoise::New<FastNoise::Simplex>();

	// CHANGE: wrapped Simplex in FractalFBm
	// plain Simplex looked too smooth, so FBm adds multiple layers of noise
	// to make the heightmap look more like terrain
	auto fnSimplex = FastNoise::New<FastNoise::FractalFBm>();
	fnSimplex->SetSource(simplex);
	fnSimplex->SetOctaveCount(5);

	// Check if the node was created successfully
	// CHANGE: also checking 'simplex'
	// if either noise node fails to initialize, the program should stop
	if (!simplex || !fnSimplex) {
		std::cerr << "Failed to create FastNoise node." << std::endl;
		return 1;
	}

	// Create a 2D vector to store noise values
	std::vector<std::vector<float>> heightmap(height, std::vector<float>(width));

	std::cout << "Generating noise..." << std::endl;

	// Generate noise using the correct API
	std::vector<float> noiseOutput(width * height);

	// GenUniformGrid2D parameters: output, xStart, yStart, xSize, ySize, frequency

	// CHANGE: using bigger step values (4.0f, 4.0f) instead of tiny values
	//  tiny steps were sampling too small an area of the noise field,
	// which created a smooth gradient instead of visible terrain variation
	fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, width, height, 4.0f, 4.0f, 1337);

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

	// CHANGE: moved range calculation outside the loops
	// the range stays constant, so there is no need to recalculate it
	// for every single pixel
	float range = maxVal - minVal;
	if (range == 0.0f) range = 1.0f;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float noise = heightmap[y][x];

			// Normalize to 0-1 range based on actual min/max
			float normalized = (noise - minVal) / range;

			// CHANGE: clamped normalized values
			// bc it protects against values going slightly outside 0-1
			if (normalized < 0.0f) normalized = 0.0f;
			if (normalized > 1.0f) normalized = 1.0f;

			// Convert to 0-255 pixel value
			unsigned char pixel = static_cast<unsigned char>(normalized * 255.0f);

			// Store in 1D pixel array
			pixels[y * width + x] = pixel;
		}
	}

	std::cout << "Saving image..." << std::endl;

	// Save as PNG file
	int result = stbi_write_png("heightmap.png", width, height, 1, pixels.data(), width);

	if (result) {
		std::cout << "Success! Heightmap saved as 'heightmap.png'" << std::endl;

		// CHANGE: corrected displayed save location
		// the file is saved to the current working directory, not build/Debug
		std::cout << "Location: heightmap.png" << std::endl;
	}
	else {
		std::cout << "ERROR: Failed to save image!" << std::endl;
	}

	std::cout << "\nPress Enter to exit..." << std::endl;
	std::cin.get();

	return 0;
}