#pragma once

#include "Forward.hpp"

#include <vulkan/vulkan_core.h>

#include <span>
#include <tuple>
#include <vector>

union QueueFamilyIndexMap {
  struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
  };

  uint32_t families[2];
};

struct PhysicalDevice {
  PhysicalDevice() : physicalDevice(VK_NULL_HANDLE) {}
  PhysicalDevice(VkPhysicalDevice physicalDevice)
      : physicalDevice(physicalDevice) {}

  operator VkPhysicalDevice() const { return physicalDevice; }

  VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const;
  std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkSurfaceKHR surface) const;
  std::vector<VkPresentModeKHR> getSurfacePresentModes(
      VkSurfaceKHR surface) const;

  VkPhysicalDevice physicalDevice;
};

std::tuple<PhysicalDevice, QueueFamilyIndexMap> pickPhysicalDevice(
    Instance const& instance,
    VkSurfaceKHR surface,
    std::span<char const* const> const& deviceExtensions);
