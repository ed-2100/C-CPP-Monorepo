#pragma once

#include "VulkanUtils.hpp"

#include <SDL3/SDL_video.h>
#include <vulkan/vulkan_core.h>

#include <memory>

namespace vke {

// ----- SDLContext -----

class SDLContext {
    class Inner {
        Inner(Inner const&) = delete;
        Inner& operator=(Inner const&) = delete;

        Inner(Inner&&) = delete;
        Inner& operator=(Inner&&) = delete;

    public:
        Inner();
        ~Inner();

        static std::shared_ptr<Inner> getInstance();
    };

    const std::shared_ptr<Inner> inner;

public:
    inline SDLContext() : inner(Inner::getInstance()) {}

    std::span<char const* const> getInstanceExtensions() const;
};

// ----- SDLWindow -----

class SDLWindow {
    class Inner {
        const SDLContext sdl_context;
        SDL_Window* const handle;

        Inner(Inner&) = delete;
        Inner& operator=(Inner&) = delete;

        Inner(Inner&&) = delete;
        Inner& operator=(Inner&&) = delete;

    public:
        Inner() = delete;
        Inner(SDLContext sdl_context, char const* name, uint32_t w, uint32_t h);
        ~Inner();

        inline operator SDL_Window*() const { return handle; }

        VkExtent2D queryExtent() const;
    };

    const std::shared_ptr<Inner> inner;

public:
    SDLWindow() = delete;
    inline SDLWindow(SDLContext sdl_context, char const* name, uint32_t w, uint32_t h)
        : inner(std::make_shared<Inner>(sdl_context, name, w, h)) {}

    inline operator SDL_Window*() const { return *inner; }

    inline VkExtent2D queryExtent() const { return inner->queryExtent(); }
};

// ----- SDLSurface -----

class SDLSurface {
    class Inner {
        const SDLWindow window;
        const Instance instance;
        const VkSurfaceKHR surface;

        Inner(Inner&) = delete;
        Inner& operator=(Inner&) = delete;

        Inner(Inner&&) = delete;
        Inner& operator=(Inner&&) = delete;

    public:
        Inner() = delete;
        Inner(SDLWindow window, Instance instance);
        ~Inner();

        inline operator VkSurfaceKHR() const { return surface; }
    };

    const std::shared_ptr<Inner> inner;

public:
    SDLSurface() = delete;
    inline SDLSurface(SDLWindow window, Instance instance)
        : inner(std::make_shared<Inner>(window, instance)) {}

    inline operator VkSurfaceKHR() const { return *inner; }
};

}  // namespace vke
