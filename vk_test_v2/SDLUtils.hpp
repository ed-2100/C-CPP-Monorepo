#pragma once

#include "VulkanUtils.hpp"

#include <SDL3/SDL_video.h>

#include <memory>

namespace vke {

// ----- SDLContext -----

struct SDLContextInner {
    SDLContextInner();
    ~SDLContextInner();

    SDLContextInner(SDLContextInner const&) = delete;
    SDLContextInner& operator=(SDLContextInner const&) = delete;

    SDLContextInner(SDLContextInner&&) = delete;
    SDLContextInner& operator=(SDLContextInner&&) = delete;

    static std::shared_ptr<SDLContextInner> getInstance();
};

class SDLContext {
    std::shared_ptr<SDLContextInner> inner;

public:
    SDLContext();

    SDLContextInner& operator*() {
        return *inner;
    }

    std::span<char const* const> getInstanceExtensions() const;
};

// ----- SDLWindow -----

struct SDLWindowInner {
    SDLWindowInner() = delete;
    SDLWindowInner(SDLContext sdl_context, SDL_Window* handle)
        : sdl_context(sdl_context), handle(handle) {}
    ~SDLWindowInner();

    SDLWindowInner(SDLWindowInner&) = delete;
    SDLWindowInner& operator=(SDLWindowInner&) = delete;

    SDLWindowInner(SDLWindowInner&&) = delete;
    SDLWindowInner& operator=(SDLWindowInner&&) = delete;

    SDLContext sdl_context;
    SDL_Window* handle;
};

class SDLWindow {
    std::shared_ptr<SDLWindowInner> inner;

public:
    SDLWindow(SDLContext sdl_context, char const* name, uint32_t w, uint32_t h);

    constexpr operator SDL_Window*() const {
        return inner->handle;
    }

    VkExtent2D queryExtent() const;
};

// ----- SDLSurface -----

struct SDLSurfaceInner {
    SDLSurfaceInner() = delete;
    SDLSurfaceInner(SDLWindow window, Instance instance);
    ~SDLSurfaceInner();

    SDLSurfaceInner(SDLSurfaceInner&) = delete;
    SDLSurfaceInner& operator=(SDLSurfaceInner&) = delete;

    SDLSurfaceInner(SDLSurfaceInner&&) = delete;
    SDLSurfaceInner& operator=(SDLSurfaceInner&&) = delete;

private:
    SDLWindow window;
    Instance instance;
    VkSurfaceKHR surface;
};

using SDLSurface = std::shared_ptr<SDLSurfaceInner>;

}  // namespace vke
