#pragma once

#include <vulkan/vulkan_core.h>

struct Instance {
  Instance() = delete;
  Instance(char const* appName, void const* pNext = nullptr);
  ~Instance();

  Instance(Instance const&) = delete;
  Instance& operator=(Instance const&) = delete;

  Instance(Instance const&&) = delete;
  Instance& operator=(Instance const&&) = delete;

  constexpr operator VkInstance() const { return instance; }

  VkInstance instance;
};
