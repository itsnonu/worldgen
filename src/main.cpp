#include <iostream>
#include <vector>
#include <algorithm> // for std::min/max
#include <FastNoise/FastNoise.h>

// UI and Graphics Headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// Image Export
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    // 1. Initialize GLFW & Window
    if (!glfwInit()) return -1;
    
    // Create the main application window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "WorldGen - Senior Capstone", NULL, NULL);
    if (!window) { 
        glfwTerminate(); 
        return -1; 
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync to prevent screen tearing

    // 2. Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 3. Generation Variables (Persistent for UI interaction)
    static int width = 256;
    static int height = 256;
    static int seed = 1337;
    static float frequency = 4.0f;
    static int octaves = 5;

    // 4. Main Render Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- UI WINDOW SIZING LOGIC ---
        // Get the current GLFW window size to force the ImGui window to fit
        int display_w, display_h;
        glfwGetWindowSize(window, &display_w, &display_h);
        
        // Lock the ImGui window to the top-left (0,0) and stretch to full width/height
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));

        // Window flags to remove the title bar and prevent the user from moving/resizing the panel
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | 
                                        ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoCollapse;

        // --- UI CONTENT ---
        ImGui::Begin("World Generator Controls", NULL, window_flags);
        
        ImGui::Text("World Generator - C++ Middleware Tool");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputInt("Seed", &seed);
        ImGui::SliderFloat("Frequency", &frequency, 0.1f, 10.0f);
        ImGui::SliderInt("Octaves", &octaves, 1, 10);

        ImGui::Spacing();
        if (ImGui::Button("Generate and Save PNG", ImVec2(200, 40))) {
            std::cout << "Generating noise with Seed: " << seed << "..." << std::endl;

            // FastNoise2 Generation Logic
            auto simplex = FastNoise::New<FastNoise::Simplex>();
            auto fnSimplex = FastNoise::New<FastNoise::FractalFBm>();
            fnSimplex->SetSource(simplex);
            fnSimplex->SetOctaveCount(octaves);

            if (simplex && fnSimplex) {
                std::vector<float> noiseOutput(width * height);
                fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, width, height, frequency, frequency, seed);

                // Find Min/Max for proper normalization
                float minVal = noiseOutput[0];
                float maxVal = noiseOutput[0];
                for (float v : noiseOutput) {
                    if (v < minVal) minVal = v;
                    if (v > maxVal) maxVal = v;
                }

                float range = (maxVal - minVal <= 0.0f) ? 1.0f : (maxVal - minVal);
                std::vector<unsigned char> pixels(width * height);

                // Convert noise to 0-255 grayscale pixels
                for (int i = 0; i < noiseOutput.size(); i++) {
                    float normalized = (noiseOutput[i] - minVal) / range;
                    pixels[i] = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, normalized)) * 255.0f);
                }

                // Save to working directory (build folder or project root)
                if (stbi_write_png("heightmap.png", width, height, 1, pixels.data(), width)) {
                    std::cout << "Success! Saved to heightmap.png" << std::endl;
                }
            }
        }

        ImGui::End();

        // 5. Final Graphics Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 6. Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}