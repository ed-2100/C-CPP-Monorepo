#pragma once

#include <vulkan/vulkan_core.h>
#include "Device.hpp"
#include "SDLUtils.hpp"

struct Swapchain {
  Swapchain() = delete;
  ~Swapchain();

  Device const& device;
  SDLSurface const& surface;
};
