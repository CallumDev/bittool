// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#include <stdint.h>
#include "imgui_impl_opengl3_loader.h"
#include "cousine.cpp"

// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("BitTool", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
    ImFont* fontSmall = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 15.0);
    ImFont* fontMedium = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 20.0);
    ImFont* fontLarge = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 25.0);
    ImFont* fontXL = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 30.0);

    ImFont* fonts[] = {
        fontSmall,
        fontMedium,
        fontLarge,
        fontXL
    };
    const char *fontLabels[] = {
        "Font Size: Small",
        "Font Size: Medium",
        "Font Size: Large",
        "Font Size: X-Large"
    };
    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;

    uint64_t num64 = 0x0;
    int bitSizeSelected = 0;
    int bitSizeActive = 0;
    int bitCount = 64;
    int fontSize = 0;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(fonts[fontSize]);
        //
        {

            int windowWidth = 0;
            int windowHeight = 0;
            SDL_GL_GetDrawableSize(window, &windowWidth, &windowHeight);

            ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
            ImGui::Begin("##mainWindow", NULL, 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_MenuBar);                         
            if(ImGui::BeginMenuBar()) {
                    if(ImGui::BeginMenu(fontLabels[fontSize])) {
                    if(ImGui::MenuItem("Small", NULL, fontSize == 0)) fontSize = 0;
                    if(ImGui::MenuItem("Medium", NULL, fontSize == 1)) fontSize = 1;
                    if(ImGui::MenuItem("Large", NULL, fontSize == 2)) fontSize = 2;
                    if(ImGui::MenuItem("X-Large", NULL, fontSize == 3)) fontSize = 3;
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            const char* bitSizes[] = {
                "64 bits",
                "32 bits",
                "16 bits",
                "8 bits"
            };
            ImGui::Combo("##bitSize", &bitSizeSelected, bitSizes, 4, 4);
            if(bitSizeSelected != bitSizeActive) {
                bitSizeActive = bitSizeSelected;
                switch(bitSizeSelected) {
                    default:
                    case 0:
                        bitCount = 64;
                        break;
                    case 1:
                        bitCount = 32;
                        break;
                    case 2:
                        bitCount = 16;
                        break;
                    case 3:
                        bitCount = 8;
                        break;
                }
            }
            const char* formats[] = {
                "%016lX",
                "%08X",
                "%04X",
                "%02X"
            };
            ImGui::InputScalar("Number", ImGuiDataType_U64, &num64, NULL, NULL, formats[bitSizeActive], ImGuiInputTextFlags_CharsHexadecimal);

            switch(bitSizeActive) {
                default:
                case 0:
                    ImGui::Text("Decimal: %llu", num64);
                    ImGui::Text("Signed: %lld", (int64_t)num64);
                    break;
                case 1:
                    num64 &= 0xFFFFFFFF;
                    ImGui::Text("Decimal: %u", (uint32_t)num64);
                    ImGui::Text("Signed: %d", (int32_t)num64);
                    break;
                case 2:
                    num64 &= 0xFFFF;
                    ImGui::Text("Decimal: %hu", (uint16_t)num64);
                    ImGui::Text("Signed: %hd", (int16_t)num64);
                    break;
                case 3:
                    num64 &= 0xFF;
                    ImGui::Text("Decimal: %hhu", (uint8_t)num64);
                    ImGui::Text("Signed: %hhd", (int8_t)num64);
                    break;    
            }
            

            int t1Offset = 0;
            if(bitCount > 32) {
                t1Offset = 32;
            }
            #define SET_COLOR(index) { \
                int colorIndex = (index / 4) % 2; \
                int c = colorIndex ? 0xFF3D3D3D : 0xFF616161; \
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, c, ImGui::TableGetColumnIndex()); \
            }
            
            if(ImGui::BeginTable("table1", bitCount - t1Offset)) {
                for(int i = (bitCount - 1); i >= t1Offset; i--) {
                    ImGui::TableNextColumn();
                    SET_COLOR(i);
                    ImGui::Text("%d", i);
                }
                for(int i = (bitCount - 1); i >= t1Offset; i--) {
                    ImGui::TableNextColumn();
                    SET_COLOR(i);
                    ImGui::PushID(i);
                    bool displayVal = ((num64 & (1ULL << i)) != 0);
                    bool isSet = displayVal;
                    if(ImGui::Checkbox("##box", &displayVal)) {
                        if(isSet) num64 &= ~(1ULL << i);
                        else num64 |= (1ULL << i);
                    }
                    ImGui::PopID();
                }
                for(int i = (bitCount - 1); i >= t1Offset; i--) {
                    ImGui::TableNextColumn();
                    SET_COLOR(i);
                    if((i + 1) % 4 == 0 && i) {
                        uint64_t mask = (1ULL << i) |
                                   (1ULL << (i - 1)) |
                                   (1ULL << (i - 2)) |
                                   (1ULL << (i - 3));
                        uint64_t n = (num64 & mask) >> (i - 3);
                        ImGui::Text("%x",n);
                    }
                }
                ImGui::EndTable();
            }

            if(bitCount > 32 && ImGui::BeginTable("table2", 32)) {
                for(int i = (32 - 1); i >= 0; i--) {
                    ImGui::TableNextColumn();
                    SET_COLOR(i);
                    ImGui::Text("%d", i);
                }
                for(int i = (32 - 1); i >= 0; i--) {
                    ImGui::TableNextColumn();
                    SET_COLOR(i);
                    ImGui::PushID(i);
                    bool displayVal = ((num64 & (1ULL << i)) != 0);
                    bool isSet = displayVal;
                    if(ImGui::Checkbox("##box", &displayVal)) {
                        if(isSet) num64 &= ~(1ULL << i);
                        else num64 |= (1ULL << i);
                    }
                    ImGui::PopID();
                }
                for(int i = (32 - 1); i >= 0; i--) {
                    ImGui::TableNextColumn();
                    SET_COLOR(i);
                    if((i + 1) % 4 == 0 && i) {
                        uint64_t mask = (1ULL << i) |
                                   (1ULL << (i - 1)) |
                                   (1ULL << (i - 2)) |
                                   (1ULL << (i - 3));
                        uint64_t n = (num64 & mask) >> (i - 3);
                        ImGui::Text("%x",n);
                    }
                }
                ImGui::EndTable();
            }
            char buf[128];
            const char* formatsSigned[] = {
                "%lld",
                "%d",
                "%hd",
                "%hhd"
            };
            if(ImGui::Button("Copy Hex")) {
                int w = snprintf(buf, 128, "%llx", num64);
                buf[w] = 0;
                SDL_SetClipboardText(buf);
            }
            ImGui::SameLine();
            if(ImGui::Button("Copy Signed")) {
                int w = snprintf(buf, 128, formatsSigned[bitSizeActive], num64);
                buf[w] = 0;
                SDL_SetClipboardText(buf);
            }
            ImGui::SameLine();
            if(ImGui::Button("Copy Unsigned")) {
                int w = snprintf(buf, 128, "%llu", num64);
                buf[w] = 0;
                SDL_SetClipboardText(buf);
            }
            ImGui::SameLine();

            ImGui::End();
        }

        ImGui::PopFont();
        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
