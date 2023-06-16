#include "MandelbrotSetGenerator.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer.h>
#include <SDL.h>
#include <iostream>
#include <functional>

void drawUI(float width, float height) {
    ImGui::SetNextWindowSize({200.0f, height});
    ImGui::SetNextWindowPos({width - 200.0f, 0.0f});
    ImGui::Begin("_", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
    ImGui::End();
    ImGui::EndFrame();
}

using SDLWindowPtr = std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>>;
using SDLRendererPtr = std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>>;
using SDLSurfacePtr = std::unique_ptr<SDL_Surface, std::function<void(SDL_Surface*)>>;
using SDLTexturePtr = std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture*)>>;

int main(int , char* []) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Unable to init SDL!");
        return -1;
    }
    int winWidth = 1280;
    int winHeight = 720;
    SDL_WindowFlags windowFlags = static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    SDLWindowPtr window(SDL_CreateWindow("",
                                         SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED,
                                         winWidth, winHeight,
                                         windowFlags),
                        SDL_DestroyWindow);

    SDLRendererPtr renderer(SDL_CreateRenderer(window.get(),
                                               -1,
                                               SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED),
                            SDL_DestroyRenderer);
    if (renderer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Error creating SDL_Renderer!");
        return -2;
    }

    int rendererWidth = 0;
    int rendererHeight = 0;
    SDL_GetRendererOutputSize(renderer.get(), &rendererWidth, &rendererHeight);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renderer resolution %dx%d",
                rendererWidth, rendererHeight);
    float widthScale = 1.0;
    float heightScale = 1.0;
    if (rendererWidth != winWidth) {
        widthScale = static_cast<float>(rendererWidth) / static_cast<float>(winWidth);
        heightScale = static_cast<float>(rendererHeight) / static_cast<float>(winHeight);
        if (widthScale != heightScale) {
            SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "widthScale != heightScale");
        }
        SDL_RenderSetScale(renderer.get(), widthScale, heightScale);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renderer scale factor: %.1fx%.1f",
                    widthScale, heightScale);
    }
    float textureWidth = rendererWidth;
    float textureHeight = rendererHeight;

    const float scale   (1/6.290223e+3);
    const float centerX(-1.186592e+0);
    const float centerY(-1.901211e-1);
    const unsigned long maxIt(350);

    MandelbrotSetGenerator drawer;
    drawer.setWidth(textureWidth);
    drawer.setHeight(textureHeight);
    drawer.setCenter(centerX, centerY);
    drawer.setScale(scale);
    drawer.setMaxIterations(maxIt);

    auto rawImage = drawer.getImage();
    SDLSurfacePtr surface(SDL_CreateRGBSurfaceFrom(rawImage.get(),
                                                   textureWidth,
                                                   textureHeight,
                                                   32, 0,
                                                   0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF),
                          SDL_FreeSurface);

    SDLTexturePtr texture(SDL_CreateTextureFromSurface(renderer.get(), surface.get()),
                          SDL_DestroyTexture);
    SDL_UpdateTexture(texture.get(), NULL, rawImage.get(), textureWidth * 4);
    SDL_Rect destRect;
    destRect.x = 10;
    destRect.y = 10;
    destRect.w = rendererWidth / 2 - 20;
    destRect.h = rendererHeight / 2 - 20;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
    ImGui_ImplSDLRenderer_Init(renderer.get());

    ImVec4 bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.get()))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        drawUI(rendererWidth, rendererHeight);

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer.get(), widthScale, heightScale);
        SDL_SetRenderDrawColor(renderer.get(),
                               static_cast<Uint8>(bgColor.x * 255),
                               static_cast<Uint8>(bgColor.y * 255),
                               static_cast<Uint8>(bgColor.z * 255),
                               static_cast<Uint8>(bgColor.w * 255));
        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), texture.get(), nullptr, &destRect);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer.get());
    }

    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();
    return 0;
}
