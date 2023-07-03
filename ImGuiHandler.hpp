#pragma once
#include "SDLTypes.hpp"
#include <imgui.h>
#include <Eigen/Dense>

class ImGuiHandler final {
public:
    ImGuiHandler(SDLWindowPtr window, SDLRendererPtr renderer);
    ~ImGuiHandler();
    void setSize(const Eigen::Vector2i& size);
    void draw();
    void render();
    void processEvent(SDL_Event *event);
    bool fullScreen() const;
    void setFullScreen(bool fullScreen);
private:
    void renderPanel();
private:
    SDLWindowPtr window_;
    SDLRendererPtr renderer_;
    Eigen::Vector2i size_;
    double scale_;
    Eigen::Vector2d center_;
    unsigned long long maxIt_;
    bool fullScreen_;
};
