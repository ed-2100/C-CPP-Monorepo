#include "VulkanUtils.hpp"

#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static void checkResult(VkResult result) {
    if (result) {
        throw std::runtime_error(
            std::format("Encountered vulkan error: {}", static_cast<int>(result))
        );
    }
}

namespace vke {

InstanceInner::InstanceInner(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
    : instance(instance), debug_messenger(debug_messenger) {
    vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT
    )vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (!vkDestroyDebugUtilsMessengerEXT) {
        checkResult(VK_ERROR_EXTENSION_NOT_PRESENT);
    }
}

InstanceInner::~InstanceInner() noexcept {
    if (debug_messenger) {
        vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
    }

    if (instance) {
        vkDestroyInstance(instance, nullptr);
    }
}

Instance InstanceBuilder::build() {
    auto app_info = VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
        .pEngineName = "Vulkan Engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    auto debug_create_info = VkDebugUtilsMessengerCreateInfoEXT{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = pNext,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };

    if (validation_layers) {
        pNext = &debug_create_info;
    }

    auto create_info = VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = pNext,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkInstance instance;
    checkResult(vkCreateInstance(&create_info, nullptr, &instance));

    VkDebugUtilsMessengerEXT debug_messenger = nullptr;
    if (validation_layers) {
        debug_create_info.pNext = nullptr;

        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT
        )vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (!vkCreateDebugUtilsMessengerEXT) {
            checkResult(VK_ERROR_EXTENSION_NOT_PRESENT);
        }

        checkResult(
            vkCreateDebugUtilsMessengerEXT(instance, &debug_create_info, nullptr, &debug_messenger)
        );
    }

    return std::make_shared<InstanceInner>(instance, debug_messenger);
}

InstanceBuilder& InstanceBuilder::with_validation_layers() {
    this->validation_layers = true;
    layers.push_back("VK_LAYER_KHRONOS_validation");
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return *this;
}

InstanceBuilder& InstanceBuilder::with_extensions(std::span<const char* const> extensions) {
    for (const auto& extension : extensions) {
        this->extensions.push_back(extension);
    }
    return *this;
}

InstanceBuilder& InstanceBuilder::with_layers(std::span<const char* const> layers) {
    for (const auto& layer : layers) {
        this->layers.push_back(layer);
    }
    return *this;
}

}  // namespace vke
