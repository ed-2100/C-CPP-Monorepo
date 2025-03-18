#include "SDLUtils.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

#include <stdexcept>

SDLContext::SDLContext() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("Failed to init SDL: ") + SDL_GetError());
    }
}

SDLContext::~SDLContext() {
    SDL_Quit();
}

std::shared_ptr<SDLContext> SDLContext::getInstance() {
    static std::weak_ptr<SDLContext> weakInstance;

    auto sharedInstance = weakInstance.lock();
    if (!sharedInstance) {
        sharedInstance = std::make_shared<SDLContext>();
        weakInstance = sharedInstance;
    }

    return sharedInstance;
}

std::span<char const* const> SDLContext::getInstanceExtensions() const {
    uint32_t sdlExtensionCount;
    char const* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
    if (nullptr == sdlExtensions) {
        throw std::runtime_error(
            std::string("Failed to get SDL's required extensions: ") + SDL_GetError()
        );
    }
    return std::span(sdlExtensions, sdlExtensionCount);
}

SDLWindow::SDLWindow(char const* name, uint32_t w, uint32_t h) {
    handle = SDL_CreateWindow(name, w, h, SDL_WINDOW_VULKAN);
}

SDLWindow::~SDLWindow() {
    if (handle) { SDL_DestroyWindow(handle); }
}

SDLSurface::SDLSurface(SDLWindow const& window, vk::Instance instance)
    : Surface(createSurface(window, instance)), instance(instance) {}

SDLSurface::~SDLSurface() {
    if (surface) { SDL_Vulkan_DestroySurface(vk::Instance(instance), surface, nullptr); }
}

vk::SurfaceKHR SDLSurface::createSurface(SDLWindow const& window, vk::Instance instance) {
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        throw std::runtime_error(std::string("Failed to create surface: ") + SDL_GetError());
    }
    return surface;
}
