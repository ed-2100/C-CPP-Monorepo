#include "Engine.hpp"

#include <iostream>
#include "SDLUtils.hpp"

namespace vke {

VulkanEngine::VulkanEngine() {
    auto sdl_context = SDLContext();

    auto window = std::make_shared<SDLWindowInner>(sdl_context, "Vulkan Engine", 500, 500);

    auto instance = InstanceBuilder()
                        .with_validation_layers()
                        .with_extensions(sdl_context.getInstanceExtensions())
                        .build();

    auto surface = std::make_shared<SDLSurfaceInner>(window, instance);

    inner = std::make_shared<VulkanEngineInner>(sdl_context, window, instance, surface);
}

void VulkanEngine::run() {
    std::cout << "Running." << std::endl;

    std::cout << "Stopped." << std::endl;
}

}  // namespace vke
