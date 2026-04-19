#include <iostream>
#include <vector>
#include <algorithm> // for std::min/max

// UI and Graphics Headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// stb_image for loading the PNG into an OpenGL texture for preview
// NOTE: STB_IMAGE_WRITE_IMPLEMENTATION is defined in main.cpp - do NOT add it here
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

// shared header - gives ui.cpp access to WorldGenConfig and GenerateWorld()
#include "../include/worldgen.h"

// All possible states for the app
enum class AppState {
    MainMenu,
    NewMap,
    LoadMap,
    Settings
};

// Loads a saved PNG from disk into a GPU texture so ImGui can display it
// Returns the OpenGL texture ID, or 0 if it failed
GLuint LoadTextureFromFile(const char* filename) {
    int w, h, channels;
    unsigned char* data = stbi_load(filename, &w, &h, &channels, 4); // force RGBA
    if (!data) return 0;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return texID;
}

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
    colors[ImGuiCol_WindowBg] = ImVec4(0.96f, 0.94f, 0.92f, 1.0f); // Cream
    colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // White
    colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // White dropdowns

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.07f, 0.10f, 0.16f, 1.0f); // Dark Navy

    // Frames (Inputs, Combo boxes)
    colors[ImGuiCol_FrameBg] = ImVec4(0.95f, 0.96f, 0.97f, 1.0f); // Light grey
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.91f, 0.92f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.86f, 0.88f, 1.0f);

    // Global Buttons
    colors[ImGuiCol_Button] = ImVec4(0.90f, 0.91f, 0.92f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.81f, 0.83f, 1.0f);

    // Dropdown Item Highlighting (Headers)
    colors[ImGuiCol_Header] = ImVec4(0.90f, 0.91f, 0.92f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.80f, 0.81f, 0.83f, 1.0f);

    // Misc
    colors[ImGuiCol_CheckMark] = ImVec4(0.07f, 0.10f, 0.16f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.07f, 0.10f, 0.16f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.20f, 0.30f, 1.0f);
}

// Dark theme - applied when user clicks Dark in Settings
void SetupDarkStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Backgrounds
    colors[ImGuiCol_WindowBg]   = ImVec4(0.07f, 0.09f, 0.12f, 1.00f); // Deep Navy header
    colors[ImGuiCol_ChildBg]    = ImVec4(0.11f, 0.14f, 0.19f, 1.00f); // Dark Slate cards
    colors[ImGuiCol_PopupBg]    = ImVec4(0.11f, 0.14f, 0.19f, 1.00f);

    // Text - Crucial for visibility
    colors[ImGuiCol_Text]       = ImVec4(0.95f, 0.96f, 0.98f, 1.00f); // Off-white text

    // Frames (Inputs, Sliders)
    colors[ImGuiCol_FrameBg]    = ImVec4(0.16f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.28f, 0.38f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.28f, 0.33f, 0.44f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button]         = ImVec4(0.20f, 0.25f, 0.33f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.28f, 0.35f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.15f, 0.20f, 0.28f, 1.00f);

    // Maintain your custom layout properties
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
}

// ============================================================
// CHANGED: was main(), now RunUI()
// main() in main.cpp calls this to boot the window
// ============================================================
int RunUI() {
    // 1. Initialize GLFW & Window
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "WorldGen", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // 2. Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFontConfig fontConfig;
    fontConfig.GlyphOffset.y = -3.0f;
    ImFont* mainFont = io.Fonts->AddFontFromFileTTF("../src/assets/Macondo-Regular.ttf", 22.0f, &fontConfig);
    if (mainFont == NULL) {
        std::cout << "Warning: Could not load custom font. Using default." << std::endl;
    }
    SetupModernStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 3. App State
    AppState currentState = AppState::MainMenu;

    // UI State Variables
    int mapSizeItem = 0;
    const char* mapSizes[] = { "Small", "Medium", "Large" };
    int worldTypeItem = 0;
    const char* worldTypes[] = { "Desert", "Forest", "Mountain", "Alien" };
    int previewTab = 0;
    int themeTab = 0;

    // Noise Parameters (shown in UI controls)
    static int   seed = 1337;
    static float frequency = 4.0f;
    static int   octaves = 5;

    // Preview texture state
    static GLuint previewTexture = 0;
    static bool   mapGenerated = false;

    // Shared generation lambda - called by both Generate and Regenerate buttons
    // Fills WorldGenConfig from UI state, calls backend GenerateWorld(), loads result as texture
    auto runGeneration = [&]() {
        WorldGenConfig config;

        // Wire Map Size dropdown to actual pixel dimensions
        if (mapSizeItem == 0) { config.width = 256;  config.height = 256; } // Small
        if (mapSizeItem == 1) { config.width = 512;  config.height = 512; } // Medium
        if (mapSizeItem == 2) { config.width = 1024; config.height = 1024; } // Large

        // Wire the rest of the controls
        config.seed = seed;
        config.frequency = frequency;
        config.octaves = octaves;
        config.worldType = worldTypeItem; // 0=Desert, 1=Forest, 2=Mountain, 3=Alien

        // Call the backend (lives in main.cpp)
        std::vector<unsigned char> pixels;
        if (GenerateWorld(config, pixels, "heightmap.png")) {
            // Free old GPU texture if one already exists
            if (previewTexture) glDeleteTextures(1, &previewTexture);
            // Load the freshly saved PNG into GPU memory for display
            previewTexture = LoadTextureFromFile("heightmap.png");
            mapGenerated = true;
        }
        };

    // 4. Main Render Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (themeTab == 0) SetupModernStyle();
        else SetupDarkStyle();

        int display_w, display_h;
        glfwGetWindowSize(window, &display_w, &display_h);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

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

            float cardWidth = display_w * 0.6f;
            if (cardWidth < 450.0f) cardWidth = 450.0f;
            float cardHeight = 450.0f;

            ImGui::SetCursorPosX((windowWidth - cardWidth) * 0.5f);
            ImGui::BeginChild("MenuCard", ImVec2(cardWidth, cardHeight), true);

            float buttonWidth = cardWidth - 32.0f;
            ImGui::SetCursorPos(ImVec2(16, 20));

            textWidth = ImGui::CalcTextSize("Main Menu").x;
            ImGui::SetCursorPosX((cardWidth - textWidth) * 0.5f);
            ImGui::Text("Main Menu");
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetCursorPosX(16);
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

            float backBtnWidth = ImGui::CalcTextSize("Main Menu").x + (ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::SameLine(ImGui::GetWindowWidth() - backBtnWidth - 20.0f);

            // Set text color: Dark Navy for Light Mode, White for Dark Mode
            ImVec4 btnTextColor = (themeTab == 0) ? ImVec4(0.07f, 0.10f, 0.16f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, btnTextColor);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent background

            if (ImGui::Button("Main Menu")) currentState = AppState::MainMenu;

            ImGui::PopStyleColor(2);
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

            // Generate World button - CHANGED: now calls runGeneration() lambda instead of inline noise code
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.20f, 0.30f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("Generate World", ImVec2(-1, 40))) {
                runGeneration(); // CHANGED: was inline FastNoise code, now calls backend
            }
            ImGui::PopStyleColor(3);

            // OLD inline generation code - removed, now handled by runGeneration() + GenerateWorld() in main.cpp
            // auto simplex = FastNoise::New<FastNoise::Simplex>();
            // auto fnSimplex = FastNoise::New<FastNoise::FractalFBm>();
            // fnSimplex->SetSource(simplex);
            // fnSimplex->SetOctaveCount(octaves);
            // if (simplex && fnSimplex) {
            //     std::vector<float> noiseOutput(width * height);
            //     fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, width, height, frequency, frequency, seed);
            //     ... (grayscale pixel conversion)
            //     stbi_write_png("heightmap.png", width, height, 1, pixels.data(), width);
            // }

            ImGui::EndChild(); // End Controls

            ImGui::SameLine();

            // --- Right Column (Preview/Edit Area) ---
            ImGui::BeginChild("PreviewArea", ImVec2(0, 0), false);

            // --- Segmented Toggle Switch (Preview/Edit) ---
            // Choose a light-grey track for light mode, or a dark-slate track for dark mode
            ImVec4 trackColor = (themeTab == 0) ? ImVec4(0.92f, 0.93f, 0.95f, 1.0f) : ImVec4(0.16f, 0.20f, 0.28f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, trackColor);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));

            ImGui::BeginChild("ToggleContainer", ImVec2(200, 38), ImGuiChildFlags_AlwaysUseWindowPadding, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            float btnWidth = (ImGui::GetContentRegionAvail().x - 2.0f) * 0.5f;

            // Preview toggle
            bool isPreview = (previewTab == 0);
            if (isPreview) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.88f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.55f, 0.6f, 1.0f));
            }
            if (ImGui::Button("Preview", ImVec2(btnWidth, 30))) previewTab = 0;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            // Edit toggle
            bool isEdit = (previewTab == 1);
            if (isEdit) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.88f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.55f, 0.6f, 1.0f));
            }
            if (ImGui::Button("Edit", ImVec2(btnWidth, 30))) previewTab = 1;
            ImGui::PopStyleColor(3);

            ImGui::PopStyleVar(2);
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();

            ImGui::Separator();

            if (previewTab == 0) {
                // --- Preview Tab ---
                ImGui::BeginChild("GridArea", ImVec2(0, -50), true);

                // CHANGED: was just static text, now shows the generated texture
                ImVec2 avail = ImGui::GetContentRegionAvail();
                if (mapGenerated && previewTexture) {
                    // Scale image to fit while keeping it square
                    float imgSize = std::min(avail.x, avail.y);
                    ImGui::SetCursorPos(ImVec2(
                        (avail.x - imgSize) * 0.5f,
                        (avail.y - imgSize) * 0.5f
                    ));
                    ImGui::Image((ImTextureID)(intptr_t)previewTexture, ImVec2(imgSize, imgSize));
                }
                else {
                    // Placeholder until first generation
                    // OLD: ImGui::Text("< MAP PREVIEW GOES HERE >");
                    const char* msg = "Press Generate World to preview";
                    ImVec2 ts = ImGui::CalcTextSize(msg);
                    ImGui::SetCursorPos(ImVec2(
                        (avail.x - ts.x) * 0.5f,
                        (avail.y - ts.y) * 0.5f
                    ));
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), msg);
                }

                ImGui::EndChild(); // End GridArea

                // Bottom buttons (Regenerate + Save Map)
                float btnWidth1 = 120.0f;
                float btnWidth2 = 120.0f;
                float spacing = ImGui::GetStyle().ItemSpacing.x;
                float totalBtnsWidth = btnWidth1 + btnWidth2 + spacing;
                ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - totalBtnsWidth);

               // Dynamically style the Regenerate button based on the theme
                // Light mode: Light grey | Dark mode: Dark slate/navy
                ImVec4 regenBg = (themeTab == 0) ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : ImVec4(0.20f, 0.25f, 0.33f, 1.0f);
                ImVec4 regenHover = (themeTab == 0) ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(0.28f, 0.35f, 0.45f, 1.00f);

                ImGui::PushStyleColor(ImGuiCol_Button, regenBg);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, regenHover);

                if (ImGui::Button("Regenerate", ImVec2(btnWidth1, 30))) {
                    runGeneration(); // Calls the backend lambda
                }
                ImGui::PopStyleColor(2);

                ImGui::SameLine();

                // Save Map button remains dark navy with white text for high contrast
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.20f, 0.30f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (ImGui::Button("Save Map", ImVec2(btnWidth2, 30))) { /* save logic */ }
                ImGui::PopStyleColor(3);
            }
            else {
                // --- Edit Tab ---
                ImGui::BeginChild("Toolbox", ImVec2(150, 0), false);
                ImGui::Text("Toolbox");
                ImGui::Button("Select", ImVec2(-1, 30));
                ImGui::Button("Draw", ImVec2(-1, 30));
                ImGui::Button("Place Object", ImVec2(-1, 30));
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
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

            float backBtnWidth = ImGui::CalcTextSize("Main Menu").x + (ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::SameLine(ImGui::GetWindowWidth() - backBtnWidth - 20.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            if (ImGui::Button("Main Menu")) currentState = AppState::MainMenu;
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::BeginChild("LoadMapCard", ImVec2(0, 0), true);

            ImGui::BeginChild("UploadSection", ImVec2(400, 0), false);
            ImGui::Text("Upload a saved map file");
            ImGui::Spacing();

            ImGui::SetNextItemWidth(250.0f);
            ImGui::InputText("##filepath", (char*)"No file chosen", 256, ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.20f, 0.30f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Button("Browse", ImVec2(100, 0))) { /* Open File Dialog */ }
            ImGui::PopStyleColor(3);

            ImGui::EndChild();
            ImGui::SameLine();

            ImGui::BeginChild("PreviewSection", ImVec2(0, 0), true);
            const char* prevText = "Loaded map preview (placeholder)";
            ImVec2 textSize = ImGui::CalcTextSize(prevText);
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2((avail.x - textSize.x) * 0.5f, (avail.y - textSize.y) * 0.5f));
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), prevText);
            ImGui::EndChild();

            ImGui::EndChild();
            break;
        }

                              // ==========================================
                              // SETTINGS
                              // ==========================================
        case AppState::Settings: {
            ImGui::Text("Settings");

            float backBtnWidth = ImGui::CalcTextSize("Main Menu").x + (ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::SameLine(ImGui::GetWindowWidth() - backBtnWidth - 20.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            if (ImGui::Button("Main Menu")) currentState = AppState::MainMenu;
            ImGui::PopStyleColor();
            ImGui::Spacing();

            ImGui::BeginChild("SettingsCard", ImVec2(0, 0), true);
            ImGui::Text("Theme");
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.92f, 0.93f, 0.95f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));

            ImGui::BeginChild("ThemeToggleContainer", ImVec2(200, 38), ImGuiChildFlags_AlwaysUseWindowPadding, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            float btnWidth = (ImGui::GetContentRegionAvail().x - 2.0f) * 0.5f;

            // Light toggle
            bool isLight = (themeTab == 0);
            if (isLight) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.88f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.55f, 0.6f, 1.0f));
            }
            if (ImGui::Button("Light", ImVec2(btnWidth, 30))) {
                themeTab = 0;
                SetupModernStyle(); //  actually applies the light theme
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            // Dark toggle
            bool isDark = (themeTab == 1);
            if (isDark) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.07f, 0.10f, 0.16f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.88f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.55f, 0.6f, 1.0f));
            }
            if (ImGui::Button("Dark", ImVec2(btnWidth, 30))) {
                themeTab = 1;
                SetupDarkStyle(); // ADDED: actually applies the dark theme
            }
            ImGui::PopStyleColor(3);

            ImGui::PopStyleVar(2);
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();

            ImGui::EndChild();
            break;
        }
        }

        ImGui::End();

        // 5. Final Graphics Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // 6. Cleanup
    // free GPU texture on exit
    if (previewTexture) glDeleteTextures(1, &previewTexture);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
