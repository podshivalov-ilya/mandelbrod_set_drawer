#include "SDLApp.hpp"

SDLApp::SDLApp()
    : surface_(nullptr, SDL_FreeSurface),
      texture_(nullptr, SDL_DestroyTexture),
      done_(false),
      fullScreen_(false)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Unable to init SDL!");
        throw SDLAppException(-1, "Unable to init SDL!");
    }
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    initWindow();
    initRenderer();
    initMandelbrotGenerator();

    rawImage_ = drawer_.getImage();

    initSurface();
    initTexture();
    destRect_ = getDestinationRect();
    initImGui();
}

SDLApp::~SDLApp() {
    SDL_Quit();
}

void SDLApp::initWindow() {
    // TODO: get resolution of the current display
    windowRect_.w = 1280;
    windowRect_.h = 720;

    SDL_WindowFlags windowFlags = static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_.reset(SDL_CreateWindow("",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   windowRect_.w, windowRect_.h,
                                   windowFlags),
                  SDL_DestroyWindow);
}

void SDLApp::initRenderer() {
    renderer_.reset(SDL_CreateRenderer(window_.get(),
                                       -1,
                                       SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED),
                    SDL_DestroyRenderer);
    if (renderer_ == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Error creating SDL_Renderer!");
        throw SDLAppException(-2, "Error creating SDL_Renderer!");
    }
    rendererRect_ = getRendererRect();
    rendererScale_ = getRendererScale();
    updateRendererScale();
}

SDL_Rect SDLApp::getRendererRect() {
    SDL_Rect rendererRect;
    SDL_GetRendererOutputSize(renderer_.get(), &rendererRect.w, &rendererRect.h);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Renderer resolution %dx%d",
                rendererRect.w, rendererRect.h);
    return rendererRect;
}

Eigen::Vector2f SDLApp::getRendererScale() {
    Eigen::Vector2f scale{1.0f, 1.0f};
    if (rendererRect_.w != windowRect_.w || rendererRect_.h != windowRect_.h) {
        scale[0] = rendererRect_.w / windowRect_.w;
        scale[1] = rendererRect_.h / windowRect_.h;
        if (scale[0] != scale[1]) {
            SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "scale[0] != scale[1]");
        }
    }
    return scale;
}

void SDLApp::updateRendererScale() {
    SDL_RenderSetScale(renderer_.get(), rendererScale_[0], rendererScale_[1]);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Renderer scale factor: %.1fx%.1f",
                rendererScale_[0], rendererScale_[1]);
}

void SDLApp::initMandelbrotGenerator() {
    const float scale(1/6.290223e+3);
    const float centerX(-1.186592e+0);
    const float centerY(-1.901211e-1);
    const unsigned long maxIt(350);

    drawer_.setSize({rendererRect_.w, rendererRect_.h});
    drawer_.setCenter({centerX, centerY});
    drawer_.setScale(scale);
    drawer_.setMaxIterations(maxIt);
}

void SDLApp::initSurface() {
    surface_.reset(SDL_CreateRGBSurfaceFrom(rawImage_.get(),
                                            rendererRect_.w,
                                            rendererRect_.h,
                                            32, 0,
                                            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF));
}

void SDLApp::initTexture() {
    texture_.reset(SDL_CreateTextureFromSurface(renderer_.get(), surface_.get()));
    SDL_UpdateTexture(texture_.get(), NULL, rawImage_.get(), rendererRect_.w * 4);
}

void SDLApp::initImGui() {
    gui_.reset(new ImGuiHandler(window_, renderer_));
    gui_->setSize({windowRect_.w, windowRect_.h});
    gui_->setScale(drawer_.scale());
    gui_->setCenter(drawer_.center());
    gui_->setMaxIterations(drawer_.maxIterations());
}

SDL_Rect SDLApp::getDestinationRect() {
    const int margin = 5;
    SDL_Rect destRect;
    destRect.x = margin;
    destRect.y = margin;
    destRect.w = rendererRect_.w / 2 - margin * 2;
    destRect.h = rendererRect_.h / 2 - margin * 2;
    return destRect;
}

void SDLApp::exec() {
    while (!done_) {
        pollEvent();

        if (drawer_.scale() != gui_->scale()) {
            drawer_.setScale(gui_->scale());
        }
        if (drawer_.center() != gui_->center()) {
            drawer_.setCenter(gui_->center());
        }
        if (drawer_.maxIterations() != gui_->maxIterations()) {
            drawer_.setMaxIterations(gui_->maxIterations());
        }
        if (gui_->updateRequested()) {
            rawImage_ = drawer_.getImage();
            gui_->resetUpdate();

            initSurface();
            //SDL_UpdateTexture(texture_.get(), NULL, surface_->pixels, surface_->pitch);
            SDL_UpdateTexture(texture_.get(), NULL, rawImage_.get(), rendererRect_.w * 4);
            destRect_ = getDestinationRect();
        }
        gui_->render();

        SDL_RenderSetScale(renderer_.get(), rendererScale_[0], rendererScale_[1]);
        //SDL_RenderSetScale(renderer_.get(), io_.DisplayFramebufferScale.x, io_.DisplayFramebufferScale.y);

        SDL_SetRenderDrawColor(renderer_.get(),
                               static_cast<Uint8>(bgColor_[0]),
                               static_cast<Uint8>(bgColor_[1]),
                               static_cast<Uint8>(bgColor_[2]),
                               static_cast<Uint8>(bgColor_[3]));
        SDL_RenderClear(renderer_.get());
        SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, &destRect_);

        gui_->draw();
        SDL_RenderPresent(renderer_.get());

        if (fullScreen_ != gui_->fullScreen()) {
            if (gui_->fullScreen()) {
                // TODO: Select the best resolution mode
                SDL_SetWindowFullscreen(window_.get(), SDL_WINDOW_FULLSCREEN);
            } else {
                SDL_SetWindowFullscreen(window_.get(), 0);
            }
            fullScreen_ = gui_->fullScreen();
        }
    }
}

void SDLApp::pollEvent() {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        gui_->processEvent(&event);
        if (event.type == SDL_QUIT)
            done_ = true;
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window_.get()))
                done_ = true;
            else if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED &&
                     event.window.windowID == SDL_GetWindowID(window_.get())) {
                windowRect_.w = event.window.data1;
                windowRect_.h = event.window.data2;
                rendererRect_ = getRendererRect();

                drawer_.setSize({rendererRect_.w, rendererRect_.h});
                gui_->setSize({windowRect_.w, windowRect_.h});
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Size changed to %dx%d", windowRect_.w, windowRect_.h);

                rawImage_ = drawer_.getImage();

                initSurface();
                //SDL_UpdateTexture(texture_.get(), NULL, surface_->pixels, surface_->pitch);
                SDL_UpdateTexture(texture_.get(), NULL, rawImage_.get(), rendererRect_.w * 4);
                destRect_ = getDestinationRect();
            }
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL) {
                controlMode_ = true;
            }
            if (controlMode_ && event.key.keysym.sym == SDLK_F11) {
                gui_->setFullScreen(!gui_->fullScreen());
            }
        }
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL) {
                controlMode_ = false;
            }
        }
    }
}
