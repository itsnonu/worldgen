#include <iostream>
#include <vector>
#include <FastNoise/FastNoise.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
	std::cout << "WorldGen Starting..." << std::endl; // This is a simple test to ensure that FastNoiseLite is working correctly.

	// Grid dimeensions 
	const int width = 256; // 256 pixel wide
	const int height = 256; //	256 pixel tall

	std::cout << "Generating " << width << "x" << height << " heightmap..." << std::endl; // Inform the user about the dimensions of the heightmap being generated.

	// Create a FastNoise node
	// Using OpenSimplex2 noise type
	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();

	// Check if the node was created successfully
	if (!fnSimplex) { 
		std::cerr << "Failed to create FastNoise node." << std::endl;
		return 1;
	}

	// Create a 2D vector to store noise value
	std::vector<std::vector<float>> heightmap(height, std::vector<float>(width));
	
	// Grid generation loop

	std::cout << "Generating noise..." << std::endl;// Inform the user that noise generation is in progress.

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) { 
			// Adjust the scale of the noise through the divisor (50.0f in this case) to get different levels of detail in the noise pattern.
			// Smaller divisor values will create more detailed noise: more details, small features, and more variation over short distances.
			// Larger divisor values will produce smoother, more gradual variations: less detail, larger features, and more gradual changes over distance.
			float noise = fnSimplex->GenSingle2D(x / 50.0f, y / 50.0f, 1337); // The x, y, and seed values 
			heightmap[y][x] = noise; // Store the noise value in the heightmap
		}

		if (y % 50 == 0) { // Print progress every 50 rows
			std::cout << "Progress: " << y << "/" << height << std::endl;
		}
		
	}
	std::cout << "Noise generation complete!"<< std::endl;

	return 0;
}