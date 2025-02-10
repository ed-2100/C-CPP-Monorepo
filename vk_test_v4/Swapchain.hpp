#pragma once

#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "SDLUtils.hpp"

#include <vulkan/vulkan_core.h>

#include <SDL3/SDL_video.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <unordered_map>

struct Swapchain {
  Swapchain() = delete;
  ~Swapchain();

  inline void chooseSurfaceFormat() {
    surfaceFormat = [&]() {
      for (auto const& availableFormat :
           physicalDevice.getSurfaceFormats(surface)) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
          return availableFormat;
        }
      }
      throw std::runtime_error(
          "The surface does not support the desired format!");
    }();
  }

  inline void chooseSurfacePresentMode() {
    const std::unordered_map<VkPresentModeKHR, uint32_t> presentModePreference{
        {VK_PRESENT_MODE_MAILBOX_KHR, 0},
        {VK_PRESENT_MODE_FIFO_KHR, 1},
    };

    uint32_t currentRating = std::numeric_limits<uint32_t>::max();
    for (const auto& availablePresentMode :
         physicalDevice.getSurfacePresentModes(surface)) {
      auto candidateKVPairIter =
          presentModePreference.find(availablePresentMode);
      if (candidateKVPairIter == presentModePreference.cend()) continue;

      uint32_t candidateRating = candidateKVPairIter->second;
      if (candidateRating < currentRating) {
        presentMode = availablePresentMode;
        currentRating = candidateRating;
      }
    }

    if (currentRating == std::numeric_limits<uint32_t>::max()) {
      throw std::runtime_error("Could not find desired present mode!");
    }
  }

  inline void querySurfaceDetails() {
    chooseSurfaceFormat();
    chooseSurfacePresentMode();
    computeSwapchainExtent(physicalDevice.getSurfaceCapabilities(surface));
  }

  inline VkExtent2D computeSwapchainExtent(
      VkSurfaceCapabilitiesKHR const& surfaceCapabilities) {
    VkExtent2D extent = window.queryExtent();

    extent.width =
        std::max(extent.width, surfaceCapabilities.minImageExtent.width);
    extent.height =
        std::max(extent.height, surfaceCapabilities.minImageExtent.height);

    if (surfaceCapabilities.maxImageExtent.width != 0) {
      extent.width =
          std::min(extent.width, surfaceCapabilities.minImageExtent.width);
    }

    if (surfaceCapabilities.maxImageExtent.height != 0) {
      extent.height =
          std::min(extent.height, surfaceCapabilities.minImageExtent.height);
    }

    return extent;
  }

  VkSurfaceFormatKHR surfaceFormat;
  VkPresentModeKHR presentMode;

  PhysicalDevice physicalDevice;
  Device const& device;
  Window const& window;
  Surface const& surface;
};
