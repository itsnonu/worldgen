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

// All possible states for the app
enum class AppState {
    MainMenu,
    NewMap,
    LoadMap,
    Settings
};

// Style layout for the gui based on the project proposal
void SetupModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Rounding for that modern, soft look
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    
    // Spacing
    style.ItemSpacing = ImVec2(8, 12);
    style.FramePadding = ImVec2(16, 8);

    ImVec4* colors = style.Colors;
    
    // Backgrounds
    colors[ImGuiCol_WindowBg]       = ImVec4(0.96f, 0.94f, 0.92f, 1.0f); // Cream background
    colors[ImGuiCol_ChildBg]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // White cards
    
    // Text
    colors[ImGuiCol_Text]           = ImVec4(0.07f, 0.10f, 0.16f, 1.0f); // Dark Navy/Black text
    
    // Frames (Dropdowns, inputs)
    colors[ImGuiCol_FrameBg]        = ImVec4(0.95f, 0.96f, 0.97f, 1.0f); // Light grey
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.91f, 0.92f, 1.0f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
    
    // Buttons (Default to the Dark Navy from your mockup)
    colors[ImGuiCol_Button]         = ImVec4(0.07f, 0.10f, 0.16f, 1.0f); 
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.15f, 0.20f, 0.30f, 1.0f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.04f, 0.06f, 0.10f, 1.0f);
    
    // Checkmarks and Sliders
    colors[ImGuiCol_CheckMark]      = ImVec4(0.07f, 0.10f, 0.16f, 1.0f);
    colors[ImGuiCol_SliderGrab]     = ImVec4(0.07f, 0.10f, 0.16f, 1.0f);
    colors[ImGuiCol_SliderGrabActive]= ImVec4(0.15f, 0.20f, 0.30f, 1.0f);
}

int main() {
    // 1. Initialize GLFW & Window
    if (!glfwInit()) return -1;
    
    // Create the main application window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "WorldGen", NULL, NULL);
    if (!window) { 
        glfwTerminate(); 
        return -1; 
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync to prevent screen tearing

    // 2. Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* mainFont = io.Fonts->AddFontFromFileTTF("../src/assets/Macondo-Regular.ttf", 22.0f); // font and size
    if (mainFont == NULL) {
    std::cout << "Warning: Could not load custom font. Using default." << std::endl;
    }
    SetupModernStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 3. Generation Variables (Persistent for UI interaction)
    AppState currentState = AppState::MainMenu; // Start at main menu

    // UI State Variables
    int mapSizeItem = 0;
    const char* mapSizes[] = { "Small", "Medium", "Large" };
    int worldTypeItem = 0;
    const char* worldTypes[] = { "Desert", "Forest", "Mountain", "Alien" };
    int previewTab = 0; // 0 = Preview, 1 = Edit

    // FastNoise Generation Variables
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
        
        switch (currentState) {
            
            // ==========================================
            // MAIN MENU
            // ==========================================
            case AppState::MainMenu: {
                ImGui::SetCursorPosY(display_h * 0.2f);
                
                float windowWidth = ImGui::GetWindowSize().x;
                float textWidth = ImGui::CalcTextSize("WorldGen").x;
                ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
                ImGui::Text("WorldGen");
                ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                float cardWidth = 400.0f;
                float cardHeight = 350.0f;
                ImGui::SetCursorPosX((windowWidth - cardWidth) * 0.5f);
                
                ImGui::BeginChild("MenuCard", ImVec2(cardWidth, cardHeight), true);
                
                float buttonWidth = cardWidth - 32.0f; 
                ImGui::SetCursorPos(ImVec2(16, 20));
                
                textWidth = ImGui::CalcTextSize("Main Menu").x;
                ImGui::SetCursorPosX((cardWidth - textWidth) * 0.5f);
                ImGui::Text("Main Menu");
                ImGui::Spacing(); ImGui::Spacing();

                ImGui::SetCursorPosX(16);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::Button("New Map", ImVec2(buttonWidth, 40))) currentState = AppState::NewMap;
                ImGui::PopStyleColor();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.96f, 0.97f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                ImGui::SetCursorPosX(16);
                if (ImGui::Button("Load Map", ImVec2(buttonWidth, 40))) currentState = AppState::LoadMap;
                
                ImGui::SetCursorPosX(16);
                if (ImGui::Button("Settings", ImVec2(buttonWidth, 40))) currentState = AppState::Settings;
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing(); ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.88f, 0.14f, 0.14f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.20f, 0.20f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::SetCursorPosX(16);
                if (ImGui::Button("Exit", ImVec2(buttonWidth, 40))) glfwSetWindowShouldClose(window, true);
                ImGui::PopStyleColor(3);

                ImGui::EndChild();
                break;
            }

            // ==========================================
            // NEW MAP
            // ==========================================
            case AppState::NewMap: {
                ImGui::Text("New Map");
                ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0)); 
                if (ImGui::Button("Main Menu")) currentState = AppState::MainMenu;
                ImGui::PopStyleColor();
                ImGui::Spacing();

                ImGui::BeginChild("NewMapCard", ImVec2(0, 0), true);
                
                // --- Left Column (Controls) ---
                ImGui::BeginChild("Controls", ImVec2(300, 0), false);
                ImGui::Text("Map Settings");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Combo("Map Size", &mapSizeItem, mapSizes, IM_ARRAYSIZE(mapSizes));
                ImGui::Combo("World Type", &worldTypeItem, worldTypes, IM_ARRAYSIZE(worldTypes));
                
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Text("Noise Parameters");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::InputInt("Seed", &seed);
                ImGui::SliderFloat("Frequency", &frequency, 0.1f, 10.0f);
                ImGui::SliderInt("Octaves", &octaves, 1, 10);
                ImGui::Spacing(); ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
                if (ImGui::Button("Generate World", ImVec2(-1, 40))) {
                    std::cout << "Generating noise with Seed: " << seed << "..." << std::endl;

                    auto simplex = FastNoise::New<FastNoise::Simplex>();
                    auto fnSimplex = FastNoise::New<FastNoise::FractalFBm>();
                    fnSimplex->SetSource(simplex);
                    fnSimplex->SetOctaveCount(octaves);

                    if (simplex && fnSimplex) {
                        std::vector<float> noiseOutput(width * height);
                        fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, width, height, frequency, frequency, seed);

                        float minVal = noiseOutput[0];
                        float maxVal = noiseOutput[0];
                        for (float v : noiseOutput) {
                            if (v < minVal) minVal = v;
                            if (v > maxVal) maxVal = v;
                        }

                        float range = (maxVal - minVal <= 0.0f) ? 1.0f : (maxVal - minVal);
                        std::vector<unsigned char> pixels(width * height);

                        for (int i = 0; i < noiseOutput.size(); i++) {
                            float normalized = (noiseOutput[i] - minVal) / range;
                            pixels[i] = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, normalized)) * 255.0f);
                        }

                        if (stbi_write_png("heightmap.png", width, height, 1, pixels.data(), width)) {
                            std::cout << "Success! Saved to heightmap.png" << std::endl;
                        }
                    }
                }
                ImGui::PopStyleColor();
                ImGui::EndChild();

                ImGui::SameLine();

                // --- Right Column (Preview/Edit Area) ---
                ImGui::BeginChild("PreviewArea", ImVec2(0, 0), false);
                
                if (ImGui::Button(previewTab == 0 ? "** Preview **" : "Preview")) previewTab = 0;
                ImGui::SameLine();
                if (ImGui::Button(previewTab == 1 ? "** Edit **" : "Edit")) previewTab = 1;

                ImGui::Separator();

                if (previewTab == 0) {
                    ImGui::BeginChild("GridArea", ImVec2(0, -50), true);
                    ImGui::Text("< MAP PREVIEW GOES HERE >");
                    ImGui::EndChild();
                    
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 220);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                    if (ImGui::Button("Regenerate", ImVec2(100, 30))) { /* regen */ }
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
                    if (ImGui::Button("Save Map", ImVec2(100, 30))) { /* save */ }
                    ImGui::PopStyleColor();
                } else {
                    ImGui::BeginChild("Toolbox", ImVec2(150, 0), false);
                    ImGui::Text("Toolbox");
                    ImGui::Button("Select", ImVec2(-1, 30));
                    ImGui::Button("Draw", ImVec2(-1, 30));
                    ImGui::Button("Place Object", ImVec2(-1, 30));
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
                    ImGui::Button("Save Map", ImVec2(-1, 40));
                    ImGui::PopStyleColor();
                    ImGui::EndChild();
                    
                    ImGui::SameLine();
                    
                    ImGui::BeginChild("EditGridArea", ImVec2(0, 0), true);
                    ImGui::Text("Editing surface...");
                    ImGui::EndChild();
                }

                ImGui::EndChild(); // End PreviewArea
                ImGui::EndChild(); // End NewMapCard
                break;
            }

            // ==========================================
            // LOAD MAP
            // ==========================================
            case AppState::LoadMap: {
                ImGui::Text("Load Map");
                ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                if (ImGui::Button("Main Menu")) currentState = AppState::MainMenu;
                ImGui::PopStyleColor();
                
                ImGui::BeginChild("LoadMapCard", ImVec2(0, 0), true);
                ImGui::Text("Upload a saved map file");
                ImGui::Spacing();
                
                ImGui::InputText("##filepath", (char*)"No file chosen", 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
                if (ImGui::Button("Browse", ImVec2(100, 30))) { /* Open File Dialog */ }
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - 100, 150));
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Loaded map preview (placeholder)");
                ImGui::EndChild();
                break;
            }

            // ==========================================
            // SETTINGS
            // ==========================================
            case AppState::Settings: {
                ImGui::Text("Settings");
                ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                if (ImGui::Button("Main Menu")) currentState = AppState::MainMenu;
                ImGui::PopStyleColor();

                ImGui::BeginChild("SettingsCard", ImVec2(0, 0), true);
                ImGui::Text("Theme");
                ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
                ImGui::Button("Light", ImVec2(80, 40));
                ImGui::PopStyleColor();
                ImGui::SameLine();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1,1,1,1)); 
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0,0,0,1));
                if (ImGui::Button("Dark", ImVec2(80, 40))) { /* Dark mode logic */ }
                ImGui::PopStyleColor(2);
                
                ImGui::EndChild();
                break;
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