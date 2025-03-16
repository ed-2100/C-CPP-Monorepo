#pragma once

#include "SDLUtils.hpp"
#include "VulkanUtils.hpp"

#include <memory>

namespace vke {

class VulkanEngineInner {
    const SDLContext sdl_context;
    const SDLWindow window;
    const Instance instance;
    const std::optional<DebugUtilsMessengerEXT> debug_messenger;
    const SDLSurface surface;

    VulkanEngineInner(VulkanEngineInner const&) = delete;
    VulkanEngineInner& operator=(VulkanEngineInner const&) = delete;

    VulkanEngineInner(VulkanEngineInner&&) = delete;
    VulkanEngineInner& operator=(VulkanEngineInner&&) = delete;

public:
    VulkanEngineInner() = delete;
    inline VulkanEngineInner(
        SDLContext sdl_context,
        SDLWindow window,
        Instance instance,
        std::optional<DebugUtilsMessengerEXT> debug_messenger,
        SDLSurface surface
    )
        : sdl_context(sdl_context),
          window(window),
          instance(instance),
          debug_messenger(debug_messenger),
          surface(surface) {}

    void run();
};

class VulkanEngine {
    std::shared_ptr<VulkanEngineInner> inner;

public:
    VulkanEngine();

    inline VulkanEngineInner& operator*() const { return *inner; }

    inline void run() { return inner->run(); }
};

}  // namespace vke
