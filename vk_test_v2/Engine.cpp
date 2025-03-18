#include "Engine.hpp"
#include <vulkan/vulkan_core.h>

#include <cstdlib>
#include <iostream>
#include <optional>
#include "VulkanUtils.hpp"

namespace vke {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

inline VkDebugUtilsMessengerCreateInfoEXT make_debug_create_info() {
    return VkDebugUtilsMessengerCreateInfoEXT{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };
}

VulkanEngine::VulkanEngine() {
    bool validation_layers = true;

    auto sdl_context = SDLContext();

    auto window = SDLWindow(sdl_context, "Vulkan Engine", 500, 500);

    auto debug_info = make_debug_create_info();

    auto instance_builder = InstanceBuilder().with_extensions(sdl_context.getInstanceExtensions());

    if (validation_layers) {
        instance_builder = instance_builder.with_validation_layers().push_pNext(&debug_info);
    }

    auto instance = instance_builder.build();

    debug_info.pNext = nullptr;

    auto debug_messenger = validation_layers
                               ? std::make_optional(DebugUtilsMessengerEXT(instance, debug_info))
                               : std::nullopt;

    auto surface = SDLSurface(window, instance);

    // clang-format off
    auto physical_devices = PhysicalDeviceSelector()
        .with_features_struct(VkPhysicalDeviceVulkan12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .descriptorIndexing = true,
            .bufferDeviceAddress = true,
        })
        .with_features_struct(VkPhysicalDeviceVulkan13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = true,
            .dynamicRendering = true,
        })
        .select(instance);
    // clang-format on

    assert(physical_devices.size() > 0);

    // VkPhysicalDevice physical_device = physical_devices[0];

    std::cout << physical_devices.size() << std::endl;

    inner = std::make_shared<Inner>(
        sdl_context,
        window,
        instance,
        debug_messenger,
        surface
    );
}

void VulkanEngine::Inner::run() {
    std::cout << "Running." << std::endl;

    std::cout << "Stopped." << std::endl;
}

}  // namespace vke
