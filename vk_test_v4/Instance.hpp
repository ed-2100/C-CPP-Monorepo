#pragma once

#include "Forward.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

struct Instance {
  Instance() : instance(VK_NULL_HANDLE) {}
  Instance(char const* appName, void const* pNext = nullptr);
  ~Instance();

  Instance(Instance const&) = delete;
  Instance& operator=(Instance const&) = delete;

  Instance(Instance const&&) = delete;
  Instance& operator=(Instance const&&) = delete;

  std::vector<PhysicalDevice> enumeratePhysicalDevices() const;

  constexpr operator VkInstance() const { return instance; }

  VkInstance instance;
};
