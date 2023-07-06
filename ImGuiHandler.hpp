#pragma once
#include "SDLTypes.hpp"
#include <imgui.h>
#include <Eigen/Dense>

class ImGuiHandler final {
public:
    ImGuiHandler(SDLWindowPtr window, SDLRendererPtr renderer);
    ~ImGuiHandler();
    void draw();
    void render();
    void processEvent(SDL_Event *event);
    bool fullScreen() const;
    void setFullScreen(bool fullScreen);
    bool updateRequested() const;
    void resetUpdate();

    // TODO: Some universal update mechanism
    Eigen::Vector2i size() const;
    void setSize(const Eigen::Vector2i& size);
    float scale() const;
    void setScale(float s);
    Eigen::Vector2f center() const;
    void setCenter(const Eigen::Vector2f& center);
    unsigned long maxIterations() const;
    void setMaxIterations(unsigned long maxIt);
private:
    void renderPanel();
private:
    SDLWindowPtr window_;
    SDLRendererPtr renderer_;
    Eigen::Vector2i size_;
    float scale_;
    Eigen::Vector2f center_;
    unsigned long long maxIt_;
    bool fullScreen_;
    bool updateRequested_;
};
