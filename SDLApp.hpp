#pragma once
#include "SDLTypes.hpp"
#include "ImGuiHandler.hpp"
#include "MandelbrotSetGenerator.hpp"

class SDLApp final {
public:
    SDLApp();
    ~SDLApp();
    void exec();
private:
    void initWindow();
    void initRenderer();
    SDL_Rect getRendererRect();
    Eigen::Vector2f getRendererScale();
    void updateRendererScale();
    void initMandelbrotGenerator();
    void initSurface();
    void initTexture();
    void initImGui();

    void pollEvent();

    SDL_Rect getDestinationRect();
private:
    SDL_Rect windowRect_;
    SDLWindowPtr window_;
    SDL_Rect rendererRect_;
    SDLRendererPtr renderer_;
    Eigen::Vector2f rendererScale_;
    MandelbrotSetGenerator drawer_;
    RawBufferPtr rawImage_;
    SDLSurfacePtr surface_;
    SDLTexturePtr texture_;
    SDL_Rect destRect_;
    std::unique_ptr<ImGuiHandler> gui_;
    bool done_;
    bool fullScreen_;
    bool controlMode_;

    const Eigen::Vector4i bgColor_{115, 140, 153, 255};
};
