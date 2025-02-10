#pragma once

#include <SDL3/SDL_video.h>
#include <vulkan/vulkan_raii.hpp>

#include "Window.hpp"

union QueueFamilyIndexMap {
  struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
  };

  uint32_t families[2];
};

struct SwapchainManager {
public:
  SwapchainManager() = delete;
  SwapchainManager(vk::raii::PhysicalDevice const& physicalDevice,
                   vk::raii::Device const& device,
                   vk::SurfaceKHR const& surface,
                   Window const& window,
                   QueueFamilyIndexMap const& queueFamilyIndices)
      : physicalDevice(physicalDevice),
        device(device),
        surface(surface),
        queueFamilyIndices(queueFamilyIndices),
        window(window) {
    createSwapchain();
  }

  SwapchainManager(const SwapchainManager&) = delete;
  SwapchainManager& operator=(const SwapchainManager&) = delete;

  SwapchainManager(SwapchainManager&& rhs) = delete;
  SwapchainManager& operator=(SwapchainManager&& other) = delete;

  void createSwapchain(vk::SwapchainKHR oldSwapchain = nullptr);
  void recreateSwapchain();

  vk::SurfaceFormatKHR surfaceFormat;
  vk::Extent2D extent;

  vk::raii::SwapchainKHR swapchain = nullptr;

  std::vector<vk::Image> images;
  std::vector<vk::raii::ImageView> imageViews;

private:
  vk::PresentModeKHR choosePresentMode() const;
  vk::SurfaceFormatKHR chooseSurfaceFormat() const;
  vk::Extent2D chooseSwapchainExtent(
      const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) const;

  vk::raii::PhysicalDevice const& physicalDevice;
  vk::raii::Device const& device;
  vk::SurfaceKHR surface;
  QueueFamilyIndexMap const& queueFamilyIndices;
  Window const& window;
};
