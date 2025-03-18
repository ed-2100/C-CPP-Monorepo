#pragma once

#include "SDLUtils.hpp"
#include "VulkanUtils.hpp"

#include <memory>

namespace vke {

class VulkanEngine {
    class Inner {
        const SDLContext sdl_context;
        const SDLWindow window;
        const Instance instance;
        const std::optional<DebugUtilsMessengerEXT> debug_messenger;
        const SDLSurface surface;

        Inner(Inner const&) = delete;
        Inner& operator=(Inner const&) = delete;

        Inner(Inner&&) = delete;
        Inner& operator=(Inner&&) = delete;

    public:
        Inner() = delete;
        inline Inner(
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

    std::shared_ptr<Inner> inner;

public:
    VulkanEngine();

    inline Inner& operator*() const { return *inner; }

    inline void run() { return inner->run(); }
};

}  // namespace vke
