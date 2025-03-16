#pragma once

#include <memory>
#include "SDLUtils.hpp"

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
    VulkanEngineInner(
        SDLContext sdl_context,
        SDLWindow window,
        Instance instance,
        SDLSurface surface
    )
        : sdl_context(sdl_context), window(window), instance(instance), surface(surface) {}
};

class VulkanEngine {
    std::shared_ptr<VulkanEngineInner> inner;

public:
    VulkanEngine();

    VulkanEngineInner& operator*() {
        return *inner;
    }

    void run();
};

}  // namespace vke
