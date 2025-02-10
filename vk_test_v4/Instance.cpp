#include "Instance.hpp"

#include <SDL3/SDL_vulkan.h>

#include <span>
#include <stdexcept>
#include <vector>

Instance::Instance(char const* appName, void const* pNext) {
  VkApplicationInfo appInfo{
      .pApplicationName = appName,
      .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
      .pEngineName = "None",
      .engineVersion = VK_MAKE_VERSION(0, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  unsigned int sdlExtensionCount;
  char const* const* sdlExtensions =
      SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

  std::vector<const char*> requiredExtensionNames;

  requiredExtensionNames.reserve(sdlExtensionCount);
  for (auto sdlExtension : std::span(sdlExtensions, sdlExtensionCount)) {
    requiredExtensionNames.push_back(sdlExtension);
  }

  requiredExtensionNames.push_back(
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

  requiredExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  std::array enabledLayers = {"VK_LAYER_KHRONOS_validation"};

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = pNext,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = enabledLayers.size(),
      .ppEnabledLayerNames = enabledLayers.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(requiredExtensionNames.size()),
      .ppEnabledExtensionNames = requiredExtensionNames.data(),
  };

  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result != VK_SUCCESS) {
    throw std::runtime_error(
        std::string("Failed to create Instance: " + std::to_string(result)));
  }
}

Instance::~Instance() {
  if (instance != VK_NULL_HANDLE) {
    vkDestroyInstance(instance, nullptr);
  }
}
