#include "ImGuiHandler.hpp"

#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

const auto panelWidth = 200.0f;

ImGuiHandler::ImGuiHandler(SDLWindowPtr window, SDLRendererPtr renderer)
    : window_(window), renderer_(renderer),
      size_({0, 0}), scale_(0.0f),
      center_({0.0f, 0.0f}), maxIt_(0),
      fullScreen_(false), updateRequested_(false) {
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

void ImGuiHandler::setFullScreen(bool fullScreen) {
    fullScreen_ = fullScreen;
}

bool ImGuiHandler::fullScreen() const {
    return fullScreen_;
}

void ImGuiHandler::resetUpdate() {
    updateRequested_ = false;
}

bool ImGuiHandler::updateRequested() const {
    return updateRequested_;
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
    ImGui::Begin("Position", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SeparatorText("Scale");
    ImGui::PushItemWidth(-1);
    ImGui::InputFloat("##scale", &scale_, 0.0f, 0.0f, "%f");
    ImGui::SeparatorText("Center (x, y)");
    ImGui::InputFloat("##x", &center_[0], 0.0f, 0.0f, "%f");
    ImGui::InputFloat("##y", &center_[1], 0.0f, 0.0f, "%f");
    ImGui::SeparatorText("Max iterations");
    ImGui::InputScalar("##maxIt", ImGuiDataType_U64, &maxIt_);
    ImGui::PopItemWidth();
    ImGui::Separator();
    ImGui::Text("Window: %dx%d", size_[0], size_[1]);
    ImGui::Checkbox("##fullScreen", &fullScreen_);
    if (ImGui::Button("Go")) {
        updateRequested_ = true;
    }
    ImGui::End();
    ImGui::EndFrame();
}

void ImGuiHandler::processEvent(SDL_Event *event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

Eigen::Vector2i ImGuiHandler::size() const {
    return size_;
}

void ImGuiHandler::setSize(const Eigen::Vector2i& size) {
    size_ = size;
}

float ImGuiHandler::scale() const {
    return scale_;
}

void ImGuiHandler::setScale(float s) {
    scale_ = s;
}

Eigen::Vector2f ImGuiHandler::center() const {
    return center_;
}

void ImGuiHandler::setCenter(const Eigen::Vector2f& center) {
    center_ = center;
}

unsigned long ImGuiHandler::maxIterations() const {
    return maxIt_;
}

void ImGuiHandler::setMaxIterations(unsigned long maxIt) {
    maxIt_ = maxIt;
}
