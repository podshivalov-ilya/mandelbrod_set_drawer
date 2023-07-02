#pragma once
#include <SDL.h>
#include <functional>
#include <memory>
#include <exception>
#include <string>

using SDLWindowPtr = std::shared_ptr<SDL_Window>;
using SDLRendererPtr = std::shared_ptr<SDL_Renderer>;
using SDLSurfacePtr = std::unique_ptr<SDL_Surface, std::function<void(SDL_Surface*)>>;
using SDLTexturePtr = std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture*)>>;

class SDLAppException : public std::exception {
public:
    SDLAppException(int errcode, const std::string& message)
        : errcode_(errcode), message_(message) {}

    int errcode() const noexcept {
        return errcode_;
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }
private:
    int errcode_;
    std::string message_;
};
