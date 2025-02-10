#pragma once

#include <vulkan/vulkan_core.h>

#include "Forward.hpp"

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
              void* pUserData);

VkDebugUtilsMessengerCreateInfoEXT createDebugUtilsMessengerCreateInfo();

struct DebugUtilsMessenger {
  DebugUtilsMessenger() = delete;
  DebugUtilsMessenger(Instance const& instance,
                      VkDebugUtilsMessengerCreateInfoEXT const& createInfo);
  ~DebugUtilsMessenger();

  DebugUtilsMessenger(DebugUtilsMessenger const&) = delete;
  DebugUtilsMessenger& operator=(DebugUtilsMessenger const&) = delete;

  DebugUtilsMessenger(DebugUtilsMessenger const&&) = delete;
  DebugUtilsMessenger& operator=(DebugUtilsMessenger const&&) = delete;

  constexpr operator VkDebugUtilsMessengerEXT() const { return messenger; }

  Instance const& instance;
  VkDebugUtilsMessengerEXT messenger;
};
