#pragma once

#include "SDLUtils.hpp"
#include "VulkanUtils.hpp"

#include <memory>

namespace vke {

class VulkanEngineInner {
    SDLContext sdl_context;
    SDLWindow window;
    Instance instance;
    SDLSurface surface;

    VulkanEngineInner(VulkanEngineInner const&) = delete;
    VulkanEngineInner& operator=(VulkanEngineInner const&) = delete;

    VulkanEngineInner(VulkanEngineInner&&) = delete;
    VulkanEngineInner& operator=(VulkanEngineInner&&) = delete;

public:
    VulkanEngineInner() = delete;
    constexpr VulkanEngineInner(
        SDLContext sdl_context,
        SDLWindow window,
        Instance instance,
        SDLSurface surface
    )
        : sdl_context(sdl_context), window(window), instance(instance), surface(surface) {}

    void run();
};

class VulkanEngine {
    std::shared_ptr<VulkanEngineInner> inner;

public:
    VulkanEngine();

    inline VulkanEngineInner& operator*() { return *inner; }

    inline void run() { return inner->run(); }
};

}  // namespace vke
