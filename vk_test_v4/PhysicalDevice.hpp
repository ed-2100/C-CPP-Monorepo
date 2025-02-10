#pragma once

#include <vulkan/vulkan_core.h>

#include <span>
#include <tuple>

union QueueFamilyIndexMap {
  struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
  };

  uint32_t families[2];
};

std::tuple<VkPhysicalDevice, QueueFamilyIndexMap> pickPhysicalDevice(
    VkInstance instance,
    VkSurfaceKHR surface,
    std::span<char const* const> const& deviceExtensions);
