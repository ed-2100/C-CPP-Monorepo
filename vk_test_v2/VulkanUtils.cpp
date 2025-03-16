#include "VulkanUtils.hpp"
#include <vulkan/vulkan_core.h>
#include <iostream>

static void checkResult(VkResult result) {
    if (result) {
        throw std::runtime_error(
            std::format("Encountered vulkan error: {}", static_cast<int>(result))
        );
    }
}

namespace vke {

// ----- Instance -----

InstanceInner::InstanceInner(const VkInstanceCreateInfo& create_info)
    : instance([&create_info]() {
          VkInstance instance;
          checkResult(vkCreateInstance(&create_info, nullptr, &instance));

          return instance;
      }()),
      vkCreateDebugUtilsMessengerEXT((PFN_vkCreateDebugUtilsMessengerEXT
      )vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")),
      vkDestroyDebugUtilsMessengerEXT((PFN_vkDestroyDebugUtilsMessengerEXT
      )vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")) {}

InstanceInner::~InstanceInner() {
    vkDestroyInstance(instance, nullptr);
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

    auto create_info = VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = pNext,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    return std::make_shared<InstanceInner>(create_info);
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

DebugUtilsMessengerEXTInner::DebugUtilsMessengerEXTInner(
    Instance instance,
    const VkDebugUtilsMessengerCreateInfoEXT& create_info
)
    : instance(instance), debug_messenger([&instance, &create_info]() {
          if (!instance->vkCreateDebugUtilsMessengerEXT ||
              !instance->vkDestroyDebugUtilsMessengerEXT) {
              checkResult(VK_ERROR_EXTENSION_NOT_PRESENT);
          }

          VkDebugUtilsMessengerEXT debug_messenger;
          checkResult(instance->vkCreateDebugUtilsMessengerEXT(
              instance,
              &create_info,
              nullptr,
              &debug_messenger
          ));

          return debug_messenger;
      }()) {}

DebugUtilsMessengerEXTInner::~DebugUtilsMessengerEXTInner() {
    instance->vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
}

}  // namespace vke
