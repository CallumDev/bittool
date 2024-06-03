#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#define strcasecmp _stricmp
#endif

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#include <stdint.h>
#include <string.h>
#include "imgui_impl_opengl3_loader.h"
#include "ini.h"
#include "cousine.cpp"
#include "DroidSansFallback.h"
#include "lang.h"

void IntegerTab();
void FloatTab();

union d2l {
    double d;
    uint64_t l;
};

union f2i {
    float f;
    uint32_t i;
};

d2l doubleedit;
f2i floatedit;

static int fontSize = 0;
static int colors = 0;
static int set_windowWidth = 930;
static int set_windowHeight = 340;
static int set_maximized = 0;
static int language_index = 0;

char *ini_read_sdl(char *str, int num, void* stream)
{
    SDL_RWops* file = (SDL_RWops*)stream;
    char *s = str;
    num--;
    int first = 0;
    while(num && SDL_RWread(file, s, 1, 1)) {
        first = 1;
        num--;
        if(*s != '\n') s++;
        else break;
    }
    *s = '\0';
    if(!first) 
        return NULL;
    return str;
}

static inline int clamped_value(const char *str, int min, int max)
{
    int v = atoi(str);
    if(v < min) return min;
    if(v > max) return max;
    return v;
}

int ini_value_read(void* user, const char* section, const char* name, const char* value)
{
    if(strcasecmp(section, "config") != 0) return 1; //ignore
    if(!strcasecmp(name, "font_size")) {
        fontSize = clamped_value(value, 0, 3);
        return 1;
    }
    if(!strcasecmp(name, "colors")) {
        colors = clamped_value(value, 0, 2);
        return 1;
    }
    if(!strcasecmp(name, "width")) {
        set_windowWidth = clamped_value(value, 100, 16384);
        return 1;
    }
    if(!strcasecmp(name, "height")) {
        set_windowHeight = clamped_value(value, 100, 16384);
        return 1;
    }
    if(!strcasecmp(name, "maximized")) {
        set_maximized = clamped_value(value, 0, 1);
        return 1;
    }
    if(!strcasecmp(name, "language")) {
        for(int i = 0; i < LANGUAGE_COUNT; i++) {
            if(!strcasecmp(value, language_codes[i])) {
                language_index = i;
                break;
            }
        }
        return 1;
    }
    return 1;
}

bool ReadConfig(const char *path)
{
    SDL_RWops* file = SDL_RWFromFile(path, "r");
    if(!file) return false;
    int retval = ini_parse_stream(ini_read_sdl, (void*)file, ini_value_read, NULL);
    SDL_RWclose(file);
    return retval == 0;
}

bool WriteConfig(const char *path)
{
    SDL_RWops* file = SDL_RWFromFile(path, "w");
    if(!file) return false;
    char buf[1024];
    #define RWprintf(file, fmt, ...) { \
    int _w = snprintf(buf, 1024, (fmt), ##__VA_ARGS__); \
    SDL_RWwrite((file), buf, _w, 1);\
    }
    RWprintf(file, "[config]\n");
    RWprintf(file, "font_size = %d\n", fontSize);
    RWprintf(file, "colors = %d\n", colors);
    RWprintf(file, "width = %d\n", set_windowWidth);
    RWprintf(file, "height = %d\n", set_windowHeight);
    RWprintf(file, "maximized = %d\n", set_maximized);
    RWprintf(file, "language = %s\n", language_codes[language_index]);
    SDL_RWclose(file);
    return true;
}

static ImFont** fonts_mono;

// Main code
int main(int, char**)
{
    doubleedit.d = 1.0;
    floatedit.f = 1.0f;
    //Set Hints
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    //Configuration
    const char *prefPath = SDL_GetPrefPath("cmcging", "bittool");
    char confPath[1024];
    if(prefPath) {
        int w = snprintf(confPath, 1024, "%s%s", prefPath, "bittool.conf");
        confPath[w] = 0;
        if(w > 1023) {
            prefPath = NULL;
            fprintf(stderr, "conf path is too long - can't read/write config\n");
        } else {
            if(!ReadConfig(confPath)) {
                if(!WriteConfig(confPath)) {
                    fprintf(stderr, "Could not write config to: %s\n", confPath);
                }
            }
        }
    } else {
        fprintf(stderr, "SDL_GetPrefPath failed - can't read/write config\n");
    }
    current_language = languages[language_index];
    #ifdef __APPLE__
    // GL 3.3 core context
    const char* glsl_version = "#version 140";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    #else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    #endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("BitTool", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, set_windowWidth, set_windowHeight, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    //
    SDL_SetWindowMinimumSize(window, 100, 100);
    if(set_maximized) {
        SDL_MaximizeWindow(window);
    }
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;

    

    // Setup Dear ImGui style
    if(colors == 1) ImGui::StyleColorsDark();
    else if (colors == 2) ImGui::StyleColorsClassic();
    else ImGui::StyleColorsLight();

    ImGui::GetStyle().FrameRounding = 2;
    ImGui::GetStyle().FrameBorderSize = 1;

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

    io.Fonts->Flags |= ImFontAtlasFlags_NoMouseCursors; //save a little texture memory
    static const ImWchar range_ASCII[] = { 0x20, 0x7F, 0 };  //don't need non-ASCII chars in monospace

    ImFont* fontSmall = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 15.0, NULL, range_ASCII);
    ImFont* fontMedium = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 20.0, NULL, range_ASCII);
    ImFont* fontLarge = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 25.0, NULL, range_ASCII);
    ImFont* fontXL = io.Fonts->AddFontFromMemoryCompressedTTF(Cousine_compressed_data, Cousine_compressed_size, 30.0, NULL, range_ASCII);
  
    ImFont* droidSmall = io.Fonts->AddFontFromMemoryCompressedBase85TTF(DroidSans_compressed_data_base85, 15.0, NULL, lang_ranges);
    ImFont* droidMedium = io.Fonts->AddFontFromMemoryCompressedBase85TTF(DroidSans_compressed_data_base85, 20.0, NULL, lang_ranges);
    ImFont* droidLarge = io.Fonts->AddFontFromMemoryCompressedBase85TTF(DroidSans_compressed_data_base85, 25.0, NULL, lang_ranges);
    ImFont* droidXL = io.Fonts->AddFontFromMemoryCompressedBase85TTF(DroidSans_compressed_data_base85, 30.0, NULL, lang_ranges);
    IM_ASSERT(droidSmall != NULL);
    ImFont* _fonts_mono[] = {
        fontSmall,
        fontMedium,
        fontLarge,
        fontXL
    };
    fonts_mono = _fonts_mono;
    ImFont* fonts_sans[] = {
        droidSmall,
        droidMedium,
        droidLarge,
        droidXL
    };
    const char *fontLabels[] = {
        "Font Size: Small",
        "Font Size: Medium",
        "Font Size: Large",
        "Font Size: X-Large"
    };

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;

    
    int renderCount = 5;
    while (!done)
    {
        
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        if(renderCount <= 0) {
            SDL_WaitEvent(NULL);
        } else {
            renderCount--; 
        }
        while (SDL_PollEvent(&event))
        {
            renderCount = 5;
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.windowID == SDL_GetWindowID(window)) {
                switch(event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        done = true;
                        break;
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        set_maximized = 1;
                        break;
                    case SDL_WINDOWEVENT_RESTORED:
                        set_maximized = 0;
                        break;
                }
            }
                
            
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(fonts_sans[fontSize]);
        //
        {

            int windowWidth = 0;
            int windowHeight = 0;
            SDL_GL_GetDrawableSize(window, &windowWidth, &windowHeight);
            //this can be different from GetDrawableSize when we enable high DPI
            SDL_GetWindowSize(window, &set_windowWidth, &set_windowHeight);
            ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
            ImGui::Begin("##mainWindow", NULL, 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_MenuBar);                         
            if(ImGui::BeginMenuBar()) {
                if(ImGui::BeginMenu(STR_FONT_SIZE)) {
                    if(ImGui::MenuItem(STR_SZ_SMALL, NULL, fontSize == 0)) fontSize = 0;
                    if(ImGui::MenuItem(STR_SZ_MEDIUM, NULL, fontSize == 1)) fontSize = 1;
                    if(ImGui::MenuItem(STR_SZ_LARGE, NULL, fontSize == 2)) fontSize = 2;
                    if(ImGui::MenuItem(STR_SZ_XLARGE, NULL, fontSize == 3)) fontSize = 3;
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu(STR_THEME)) {
                    if(ImGui::MenuItem(STR_THEME_LIGHT, NULL, colors == 0)) {
                        colors = 0;
                        ImGui::StyleColorsLight();
                    }
                    if(ImGui::MenuItem(STR_THEME_DARK, NULL, colors == 1)) {
                        colors = 1;
                        ImGui::StyleColorsDark();
                    }
                    if(ImGui::MenuItem(STR_THEME_CLASSIC, NULL, colors == 2)) {
                        colors = 2;
                        ImGui::StyleColorsClassic();
                    }
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu(STR_LANGUAGE)) {
                    for(int i = 0; i < LANGUAGE_COUNT; i++) {
                        if(ImGui::MenuItem(language_names[i], NULL, language_index == i)) {
                            language_index = i;
                            current_language = languages[i];
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::BeginTabBar("##tabs");
            if(ImGui::BeginTabItem(STR_INTEGER)) {
                IntegerTab();
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem(STR_FLOAT)) {
                FloatTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();

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

    if(!WriteConfig(confPath)) {
        fprintf(stderr, "Could not write config to: %s\n", confPath);
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


void FloatTab()
{
    ImGui::Text(STR_DOUBLE_PRECISION);
    ImGui::PushFont(fonts_mono[fontSize]);
    ImGui::InputScalar("uint64_t", ImGuiDataType_U64, &doubleedit.l, NULL, NULL, "%lX", ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::InputScalar("double", ImGuiDataType_Double, &doubleedit.d, NULL, NULL);
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Text(STR_SINGLE_PRECISION);
    ImGui::PushFont(fonts_mono[fontSize]);
    ImGui::InputScalar("uint32_t", ImGuiDataType_U32, &floatedit.i, NULL, NULL, "%x", ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::InputScalar("float", ImGuiDataType_Float, &floatedit.f, NULL, NULL);
    ImGui::PopFont();
}

static uint64_t num64 = 0x0;
static int bitSizeSelected = 0;
static int bitSizeActive = 0;
static int bitCount = 64;

void BitTable(int tableIndex, int startBit, int endBit)
{
    ImGui::PushFont(fonts_mono[fontSize]);
    ImGui::PushID(tableIndex);

    #define SET_COLOR(index) { \
        int colorIndex = (index / 4) % 2; \
        ImU32 c = ImGui::GetColorU32(colorIndex ? ImGuiCol_TableRowBg : ImGuiCol_TableRowBgAlt); \
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, c, ImGui::TableGetColumnIndex()); \
    }
    
    if(ImGui::BeginTable("table", startBit - endBit + 1, ImGuiTableFlags_BordersOuter)) {
        for(int i = startBit; i >= endBit; i--) {
            ImGui::TableNextColumn();
            SET_COLOR(i);
            ImGui::Text("%d", i);
        }
        for(int i = startBit; i >= endBit; i--) {
            ImGui::TableNextColumn();
            SET_COLOR(i);
            ImGui::PushID(i);
            bool displayVal = ((num64 & (1ULL << i)) != 0);
            bool isSet = displayVal;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1,1));
            if(ImGui::Checkbox("##box", &displayVal)) {
                if(isSet) num64 &= ~(1ULL << i);
                else num64 |= (1ULL << i);
            }
            ImGui::PopStyleVar();
            ImGui::PopID();
        }
        for(int i = startBit; i >= endBit; i--) {
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
    ImGui::PopID();
    ImGui::PopFont();
}

void IntegerTab()
{
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
    ImGui::AlignTextToFramePadding();
    ImGui::Text(STR_HEX);
    ImGui::SameLine();
    ImGui::PushFont(fonts_mono[fontSize]);
    ImGui::InputScalar("##hex", ImGuiDataType_U64, &num64, NULL, NULL, formats[bitSizeActive], ImGuiInputTextFlags_CharsHexadecimal);
    float threshold32 = ImGui::CalcTextSize("00").x * 40;
    float threshold16 = ImGui::CalcTextSize("00").x * 20;
    ImGui::PopFont();

    float windowWidth = ImGui::GetWindowWidth();
    float lineHeight = ImGui::GetTextLineHeightWithSpacing();

    #define NUM_DISPLAY(txt,fmt) { \
        ImGui::Text("%s: ", txt); \
        ImGui::SameLine(); \
        ImGui::PushFont(fonts_mono[fontSize]); \
        ImGui::Text(fmt, num64); \
        ImGui::PopFont(); \
    }
    
    #define CALC_DISPLAY(txt, fmt, num) { \
        ImGui::Text("%s: ", txt); \
        ImGui::SameLine(); \
        ImGui::PushFont(fonts_mono[fontSize]); \
        if ((num) < 0) ImGui::Text("?"); \
        else ImGui::Text(fmt, (num)); \
        ImGui::PopFont(); \
    }
    
    #define POPCOUNT(x) __builtin_popcount((x))
    #define LZCNT64(x) (x == 0 ? -1 : __builtin_clzll((x)))
    #define LZCNT32(x) (x == 0 ? -1 : __builtin_clz((unsigned int)(x)))
    #define LZCNT16(x) (x == 0 ? -1 : __builtin_clz((uint16_t)(x)) - 16)
    #define LZCNT8(x) (x == 0 ? -1 : __builtin_clz((uint8_t)(x)) - 24)
    #define TZCNT64(x) (x == 0 ? -1 : __builtin_ctzll((x)))
    #define TZCNT32(x) (x == 0 ? -1 : __builtin_ctz((unsigned int)(x)))
    
    switch(bitSizeActive) {
        default:
        case 0:
            NUM_DISPLAY(STR_UNSIGNED, "%llu");
            NUM_DISPLAY(STR_SIGNED, "%lld");
            CALC_DISPLAY("popcnt", "%d", POPCOUNT(num64));
            ImGui::SameLine();
            CALC_DISPLAY("lzcnt", "%d", LZCNT64(num64));
            ImGui::SameLine();
            CALC_DISPLAY("tzcnt", "%d", TZCNT64(num64));
        break;
        case 1:
            num64 &= 0xFFFFFFFF;
            NUM_DISPLAY(STR_UNSIGNED, "%u");
            NUM_DISPLAY(STR_SIGNED, "%d");
            CALC_DISPLAY("popcnt", "%d", POPCOUNT(num64));
            ImGui::SameLine();
            CALC_DISPLAY("lzcnt", "%d", LZCNT32(num64));
            ImGui::SameLine();
            CALC_DISPLAY("tzcnt", "%d", TZCNT32(num64));
            break;
        case 2:
            num64 &= 0xFFFF;
            NUM_DISPLAY(STR_UNSIGNED, "%hu");
            NUM_DISPLAY(STR_SIGNED, "%hd");
            CALC_DISPLAY("popcnt", "%d", POPCOUNT(num64));
            ImGui::SameLine();
            CALC_DISPLAY("lzcnt", "%d", LZCNT16(num64));
            ImGui::SameLine();
            CALC_DISPLAY("tzcnt", "%d", TZCNT32(num64));
            break;
        case 3:
            num64 &= 0xFF;
            NUM_DISPLAY(STR_UNSIGNED, "%hhu");
            NUM_DISPLAY(STR_SIGNED, "%hhd");
            CALC_DISPLAY("popcnt", "%d", POPCOUNT(num64));
            ImGui::SameLine();
            CALC_DISPLAY("lzcnt", "%d", LZCNT8(num64));
            ImGui::SameLine();
            CALC_DISPLAY("tzcnt", "%d", TZCNT32(num64));
            break;
    }

    float padding = ImGui::GetStyle().FramePadding.x;
    float height = ImGui::GetWindowHeight();
    ImGui::BeginChild("##bits", ImVec2(ImGui::GetWindowWidth() - padding * 6, -(lineHeight * 1.5)), true);
    switch(bitSizeActive) {
        default:
        case 0: //64
            if(windowWidth < threshold16) {
                BitTable(0, 63, 56);
                BitTable(1, 55, 48);
                BitTable(2, 47, 40);
                BitTable(3, 39, 32);
                BitTable(4, 31, 24);
                BitTable(5, 23, 16);
                BitTable(6, 15, 8);
                BitTable(7, 7, 0);
            } else if (windowWidth < threshold32) {
                BitTable(8, 63, 48);
                BitTable(9, 47, 32);
                BitTable(10, 31, 16);
                BitTable(11, 15, 0);
            } else {
                BitTable(12, 63, 32);
                BitTable(13, 31, 0);
            }
            break;
        case 1: //32
            if(windowWidth < threshold16) {
                BitTable(14, 31, 24);
                BitTable(15, 23, 16);
                BitTable(16, 15, 8);
                BitTable(17, 7, 0);
            } else if (windowWidth < threshold32) {
                BitTable(18, 31, 16);
                BitTable(19, 15, 0);
            } else {
                BitTable(20, 31, 0);
            }
            break;
        case 2: //16
            if(windowWidth < threshold16) {
                BitTable(21, 15, 8);
                BitTable(22, 7, 0);
            } else {
                BitTable(23, 15, 0);
            }
            break;
        case 3: //8
            BitTable(24, 7, 0);
            break;    
    }
    
    ImGui::EndChild();
    
    char buf[128];
    const char* formatsSigned[] = {
        "%lld",
        "%d",
        "%hd",
        "%hhd"
    };
    if(ImGui::Button(STR_COPY_HEX)) {
        int w = snprintf(buf, 128, "%llx", num64);
        buf[w] = 0;
        SDL_SetClipboardText(buf);
    }
    ImGui::SameLine();
    if(ImGui::Button(STR_COPY_SIGNED)) {
        int w = snprintf(buf, 128, formatsSigned[bitSizeActive], num64);
        buf[w] = 0;
        SDL_SetClipboardText(buf);
    }
    ImGui::SameLine();
    if(ImGui::Button(STR_COPY_UNSIGNED)) {
        int w = snprintf(buf, 128, "%llu", num64);
        buf[w] = 0;
        SDL_SetClipboardText(buf);
    }
}
