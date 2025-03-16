#include "SDLUtils.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include "VulkanUtils.hpp"

namespace vke {

// ----- SDLContext -----

static bool sdl_initialized = false;

SDLContextInner::SDLContextInner() {
    assert(!sdl_initialized);
    sdl_initialized = true;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::format("Failed to init SDL: {}", SDL_GetError()));
    }
}

SDLContextInner::~SDLContextInner() {
    SDL_Quit();

    sdl_initialized = false;
}

std::shared_ptr<SDLContextInner> SDLContextInner::getInstance() {
    static std::weak_ptr<SDLContextInner> weakInstance;
    static std::mutex mutex;

    std::lock_guard<std::mutex> lock(mutex);
    auto sharedInstance = weakInstance.lock();
    if (!sharedInstance) {
        sharedInstance = std::make_shared<SDLContextInner>();
        weakInstance = sharedInstance;
    }

    return sharedInstance;
}

SDLContext::SDLContext() {
    inner = SDLContextInner::getInstance();
}

std::span<char const* const> SDLContext::getInstanceExtensions() const {
    uint32_t sdlExtensionCount;
    char const* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
    if (nullptr == sdlExtensions) {
        throw std::runtime_error(
            std::format("Failed to get SDL's required extensions: {}", SDL_GetError())
        );
    }
    return std::span(sdlExtensions, sdlExtensionCount);
}

// ----- SDLWindow -----

SDLWindowInner::SDLWindowInner(SDLContext sdl_context, char const* name, uint32_t w, uint32_t h)
    : sdl_context(sdl_context) {
    handle = SDL_CreateWindow(name, w, h, SDL_WINDOW_VULKAN);
    if (!handle) {
        throw std::runtime_error(std::format("Failed to create an SDL window: {}", SDL_GetError()));
    }
}

SDLWindowInner::~SDLWindowInner() {
    if (handle) {
        SDL_DestroyWindow(handle);
    }
}

VkExtent2D SDLWindowInner::queryExtent() const {
    int width, height;
    if (!SDL_GetWindowSize(handle, &width, &height)) {
        throw std::runtime_error(std::format("Failed to get query extent: {}", SDL_GetError()));
    }

    return VkExtent2D{
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height)
    };
}

// ----- SDLSurface -----

SDLSurfaceInner::SDLSurfaceInner(SDLWindow window, Instance instance)
    : window(window), instance(instance) {
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        throw std::runtime_error(std::format("Failed to create surface: {}", SDL_GetError()));
    }
}

SDLSurfaceInner::~SDLSurfaceInner() {
    if (surface) {
        SDL_Vulkan_DestroySurface(instance, surface, nullptr);
    }
}

}  // namespace vke
