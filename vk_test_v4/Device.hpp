#pragma once

#include <vulkan/vulkan_core.h>
#include <span>

struct Device {
  Device() = delete;
  Device(VkPhysicalDevice physicalDevice,
         std::span<char const* const> const& deviceExtensions,
         std::span<uint32_t const> const& queueFamilyIndices);
  ~Device();

  Device(Device const&) = delete;
  Device& operator=(Device const&) = delete;

  Device(Device const&&) = delete;
  Device& operator=(Device const&&) = delete;

  operator VkDevice() { return device; }

  VkDevice device;
};
