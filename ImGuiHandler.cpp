#include "ImGuiHandler.hpp"

#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

const auto panelWidth = 200.0f;

ImGuiHandler::ImGuiHandler(SDLWindowPtr window, SDLRendererPtr renderer)
    : window_(window), renderer_(renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window_.get(), renderer.get());
    ImGui_ImplSDLRenderer2_Init(renderer_.get());
}

ImGuiHandler::~ImGuiHandler() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiHandler::setSize(const Eigen::Vector2i& size) {
    size_ = size;
}

void ImGuiHandler::render() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    renderPanel();

    ImGui::Render();
}

void ImGuiHandler::draw() {
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiHandler::renderPanel() {
    ImGui::NewFrame();
    ImGui::SetNextWindowSize({panelWidth, static_cast<float>(size_[1])});
    ImGui::SetNextWindowPos({static_cast<float>(size_[0]) - panelWidth, 0.0f});
    ImGui::Begin("_", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("This is some useful text.");
    ImGui::End();
    ImGui::EndFrame();
}

void ImGuiHandler::processEvent(SDL_Event *event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}
