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
    
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    
    style.ItemSpacing = ImVec2(8, 12);
    style.FramePadding = ImVec2(16, 8);

    ImVec4* colors = style.Colors;
    
    // Backgrounds
    colors[ImGuiCol_WindowBg]       = ImVec4(0.96f, 0.94f, 0.92f, 1.0f); // Cream
    colors[ImGuiCol_ChildBg]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // White
    colors[ImGuiCol_PopupBg]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // White dropdowns
    
    // Text
    colors[ImGuiCol_Text]           = ImVec4(0.07f, 0.10f, 0.16f, 1.0f); // Dark Navy
    
    // Frames (Inputs, Combo boxes)
    colors[ImGuiCol_FrameBg]        = ImVec4(0.95f, 0.96f, 0.97f, 1.0f); // Light grey
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.91f, 0.92f, 1.0f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
    
    // Global Buttons (Now Light Grey so the tiny arrows are visible!)
    colors[ImGuiCol_Button]         = ImVec4(0.90f, 0.91f, 0.92f, 1.0f); 
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.80f, 0.81f, 0.83f, 1.0f);
    
    // Dropdown Item Highlighting (Headers)
    colors[ImGuiCol_Header]         = ImVec4(0.90f, 0.91f, 0.92f, 1.0f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.80f, 0.81f, 0.83f, 1.0f);
    
    // Misc
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
    ImFontConfig fontConfig;
    // Shift the text UP by 3 pixels
    fontConfig.GlyphOffset.y = -3.0f;
    ImFont* mainFont = io.Fonts->AddFontFromFileTTF("../src/assets/Macondo-Regular.ttf", 22.0f, &fontConfig); // font and size
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

                // Calculate card size based on screen width
                float cardWidth = display_w * 0.6f; // Make the card take up 60% of the screen width
                if (cardWidth < 450.0f) cardWidth = 450.0f; // But never let it get smaller than 450px
                float cardHeight = 450.0f; // Increased height to fit the new, larger font

                ImGui::SetCursorPosX((windowWidth - cardWidth) * 0.5f);
                ImGui::BeginChild("MenuCard", ImVec2(cardWidth, cardHeight), true);

                float buttonWidth = cardWidth - 32.0f; // Keep the 16px padding on both sides
                ImGui::SetCursorPos(ImVec2(16, 20));
                
                textWidth = ImGui::CalcTextSize("Main Menu").x;
                ImGui::SetCursorPosX((cardWidth - textWidth) * 0.5f);
                ImGui::Text("Main Menu");
                ImGui::Spacing(); ImGui::Spacing();

                ImGui::SetCursorPosX(16);
                // Push Dark Navy background and White text
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.20f, 0.30f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                if (ImGui::Button("New Map", ImVec2(buttonWidth, 40))) currentState = AppState::NewMap;

                ImGui::PopStyleColor(3);
                
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
                
                // Dynamically calculate the button width + padding so it never crops
                float backBtnWidth = ImGui::CalcTextSize("Main Menu").x + (ImGui::GetStyle().FramePadding.x * 2.0f);
                ImGui::SameLine(ImGui::GetWindowWidth() - backBtnWidth - 20.0f); // 20px padding from right edge
                
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
                
                // Push Dark Navy background and White text
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.20f, 0.30f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
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
                ImGui::PopStyleColor(3);
                ImGui::EndChild();

                ImGui::SameLine();

                // --- Right Column (Preview/Edit Area) ---
                ImGui::BeginChild("PreviewArea", ImVec2(0, 0), false);

                // --- Segmented Toggle Switch (Preview/Edit) ---

                // 1. Style the pill container (Light grey background, rounded edges, slight inner padding)
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.92f, 0.93f, 0.95f, 1.0f)); 
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4)); 

                // We make a tiny child window to act as the grey background "track"
                ImGui::BeginChild("ToggleContainer", ImVec2(200, 38), ImGuiChildFlags_AlwaysUseWindowPadding, 0);
                // Style the buttons inside the track
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0)); // Tiny gap between the two options
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);       // Round the inner active "switch"

                // Calculate button width dynamically so they perfectly fill the container
                float btnWidth = (ImGui::GetContentRegionAvail().x - 2.0f) * 0.5f;

                // --- PREVIEW TOGGLE ---
                bool isPreview = (previewTab == 0);
                if (isPreview) {
                    // Active: Solid white block, dark navy text
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                } else {
                    // Inactive: Totally transparent background, faded text
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.88f, 0.92f, 1.0f)); // Slight hover effect
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.55f, 0.6f, 1.0f));
                }

                if (ImGui::Button("Preview", ImVec2(btnWidth, 30))) previewTab = 0;
                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                // --- EDIT TOGGLE ---
                bool isEdit = (previewTab == 1);
                if (isEdit) {
                    // Active: Solid white block, dark navy text
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                } else {
                    // Inactive: Totally transparent background, faded text
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.88f, 0.92f, 1.0f)); // Slight hover effect
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.55f, 0.6f, 1.0f));
                }

                if (ImGui::Button("Edit", ImVec2(btnWidth, 30))) previewTab = 1;
                ImGui::PopStyleColor(3);

                // Clean up styles
                ImGui::PopStyleVar(2); // Button styling
                ImGui::EndChild();     // End toggle container
                ImGui::PopStyleVar(2); // Container styling
                ImGui::PopStyleColor(); // Container background color

                ImGui::Separator();

                if (previewTab == 0) {
                ImGui::BeginChild("GridArea", ImVec2(0, -50), true);
                ImGui::Text("< MAP PREVIEW GOES HERE >");
                ImGui::EndChild();
                
                // Dynamically right-align the two bottom buttons
                float btnWidth1 = 120.0f; // Made slightly wider for the new font
                float btnWidth2 = 120.0f;
                float spacing = ImGui::GetStyle().ItemSpacing.x;
                float totalBtnsWidth = btnWidth1 + btnWidth2 + spacing;
                
                ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - totalBtnsWidth);
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                if (ImGui::Button("Regenerate", ImVec2(btnWidth1, 30))) { /* regen */ }
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
                
                // Push Dark Navy background and White text
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.20f, 0.30f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::Button("Save Map", ImVec2(btnWidth2, 30))) { /* save */ }
                ImGui::PopStyleColor(3);
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
                
                // Dynamically right-align the Main Menu button
                float backBtnWidth = ImGui::CalcTextSize("Main Menu").x + (ImGui::GetStyle().FramePadding.x * 2.0f);
                ImGui::SameLine(ImGui::GetWindowWidth() - backBtnWidth - 20.0f);
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                if (ImGui::Button("Main Menu")) currentState = AppState::MainMenu;
                ImGui::PopStyleColor();
                ImGui::Spacing();
                
                // The main white card
                ImGui::BeginChild("LoadMapCard", ImVec2(0, 0), true);
                
                // --- Left Column (Upload Controls) ---
                ImGui::BeginChild("UploadSection", ImVec2(400, 0), false);
                ImGui::Text("Upload a saved map file");
                ImGui::Spacing();
                
                ImGui::SetNextItemWidth(250.0f);
                ImGui::InputText("##filepath", (char*)"No file chosen", 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::SameLine();
                
                // Dark Navy Browse Button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.20f, 0.30f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::Button("Browse", ImVec2(100, 0))) { /* Open File Dialog */ }
                ImGui::PopStyleColor(3);
                
                ImGui::EndChild(); // End UploadSection
                
                ImGui::SameLine();
                
                // --- Right Column (Preview Box) ---
                ImGui::BeginChild("PreviewSection", ImVec2(0, 0), true); // true = draw border
                
                // Math to perfectly center text inside this box
                const char* prevText = "Loaded map preview (placeholder)";
                ImVec2 textSize = ImGui::CalcTextSize(prevText);
                ImVec2 avail = ImGui::GetContentRegionAvail();
                ImGui::SetCursorPos(ImVec2((avail.x - textSize.x) * 0.5f, (avail.y - textSize.y) * 0.5f));
                
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), prevText);
                ImGui::EndChild(); // End PreviewSection

                ImGui::EndChild(); // End LoadMapCard
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