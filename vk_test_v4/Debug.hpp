#pragma once

#include <vulkan/vulkan_core.h>

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
              void* pUserData);

VkDebugUtilsMessengerCreateInfoEXT createDebugUtilsMessengerCreateInfo();

struct DebugMessenger {
  DebugMessenger() = delete;
  DebugMessenger(VkInstance instance,
                 VkDebugUtilsMessengerCreateInfoEXT const& createInfo);
  ~DebugMessenger();

  DebugMessenger(DebugMessenger const&) = delete;
  DebugMessenger& operator=(DebugMessenger const&) = delete;

  DebugMessenger(DebugMessenger const&&) = delete;
  DebugMessenger& operator=(DebugMessenger const&&) = delete;

  constexpr operator VkDebugUtilsMessengerEXT() const { return messenger; }

  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
};
