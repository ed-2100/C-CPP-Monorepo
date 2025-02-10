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
    queryDetails();
    createSwapchain();
    createImageViews();
  }

  SwapchainManager(const SwapchainManager&) = delete;
  SwapchainManager& operator=(const SwapchainManager&) = delete;

  SwapchainManager(SwapchainManager&& rhs) = delete;
  SwapchainManager& operator=(SwapchainManager&& other) = delete;

  void createSwapchain(vk::SwapchainKHR oldSwapchain = nullptr);
  void recreateSwapchain();
  void createImageViews();
  void queryDetails();

  static vk::PresentModeKHR choosePresentMode(
      const std::vector<vk::PresentModeKHR>& availablePresentModes);
  static vk::Extent2D computeSwapchainExtent(
      const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
      Window const& window);

  vk::raii::PhysicalDevice const& physicalDevice;
  vk::raii::Device const& device;
  vk::SurfaceKHR surface;
  QueueFamilyIndexMap const& queueFamilyIndices;
  Window const& window;

  vk::SurfaceFormatKHR surfaceFormat;
  vk::SurfaceCapabilitiesKHR surfaceCapabilities;
  vk::PresentModeKHR presentMode;
  vk::Extent2D extent;

  vk::raii::SwapchainKHR swapchain = nullptr;
  std::vector<vk::Image> images;
  std::vector<vk::raii::ImageView> imageViews;
};
