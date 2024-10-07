#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL.h>
#include "RCT_5_Control.h"
#include "fonts.h"
#include "imfilebrowser.h"
#include "implot.h"

// Main code
int main(int, char **)
{

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
#ifdef _WIN32
    FreeConsole();
#endif
    // Create window with graphics context
    SDL_Window *window = SDL_CreateWindow("RCT 5 Controller", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 18.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(CousineRegular_compressed_data, CousineRegular_compressed_size, 18.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, 18.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyClean_compressed_data, ProggyClean_compressed_size, 18.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyTiny_compressed_data, ProggyTiny_compressed_size, 18.0f);

    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 24.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(CousineRegular_compressed_data, CousineRegular_compressed_size, 24.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, 24.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyClean_compressed_data, ProggyClean_compressed_size, 24.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyTiny_compressed_data, ProggyTiny_compressed_size, 24.0f);

    RCT_5_Control rct5Control;
    // Main loop
    return rct5Control.render_window(window, io, renderer);

}
