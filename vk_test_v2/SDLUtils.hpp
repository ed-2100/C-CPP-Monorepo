#pragma once

#include "VulkanUtils.hpp"

#include <SDL3/SDL_video.h>
#include <vulkan/vulkan_core.h>

#include <memory>

namespace vke {

// ----- SDLContext -----

class SDLContextInner {
    SDLContextInner(SDLContextInner const&) = delete;
    SDLContextInner& operator=(SDLContextInner const&) = delete;

    SDLContextInner(SDLContextInner&&) = delete;
    SDLContextInner& operator=(SDLContextInner&&) = delete;

public:
    SDLContextInner();
    ~SDLContextInner();

    static std::shared_ptr<SDLContextInner> getInstance();
};

class SDLContext {
    const std::shared_ptr<SDLContextInner> inner;

public:
    inline SDLContext() : inner(SDLContextInner::getInstance()) {}

    std::span<char const* const> getInstanceExtensions() const;
};

// ----- SDLWindow -----

class SDLWindowInner {
    const SDLContext sdl_context;
    SDL_Window* const handle;

    SDLWindowInner(SDLWindowInner&) = delete;
    SDLWindowInner& operator=(SDLWindowInner&) = delete;

    SDLWindowInner(SDLWindowInner&&) = delete;
    SDLWindowInner& operator=(SDLWindowInner&&) = delete;

public:
    SDLWindowInner() = delete;
    SDLWindowInner(SDLContext sdl_context, char const* name, uint32_t w, uint32_t h);
    ~SDLWindowInner();

    inline operator SDL_Window*() const { return handle; }

    VkExtent2D queryExtent() const;
};

class SDLWindow {
    const std::shared_ptr<SDLWindowInner> inner;

public:
    SDLWindow() = delete;
    inline SDLWindow(SDLContext sdl_context, char const* name, uint32_t w, uint32_t h)
        : inner(std::make_shared<SDLWindowInner>(sdl_context, name, w, h)) {}

    inline operator SDL_Window*() const { return *inner; }

    inline VkExtent2D queryExtent() const { return inner->queryExtent(); }
};

// ----- SDLSurface -----

class SDLSurfaceInner {
    const SDLWindow window;
    const Instance instance;
    const VkSurfaceKHR surface;

    SDLSurfaceInner(SDLSurfaceInner&) = delete;
    SDLSurfaceInner& operator=(SDLSurfaceInner&) = delete;

    SDLSurfaceInner(SDLSurfaceInner&&) = delete;
    SDLSurfaceInner& operator=(SDLSurfaceInner&&) = delete;

public:
    SDLSurfaceInner() = delete;
    SDLSurfaceInner(SDLWindow window, Instance instance);
    ~SDLSurfaceInner();

    inline operator VkSurfaceKHR() const { return surface; }
};

class SDLSurface {
    const std::shared_ptr<SDLSurfaceInner> inner;

public:
    SDLSurface() = delete;
    inline SDLSurface(SDLWindow window, Instance instance)
        : inner(std::make_shared<SDLSurfaceInner>(window, instance)) {}

    inline operator VkSurfaceKHR() const { return *inner; }
};

}  // namespace vke
