#pragma once

#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

enum QueueFamily {
  graphicsFamily,
  presentFamily,
  count, // SAFETY: Must be at the end of the enum.
};

using QueueFamilyIndexMap =
    std::array<uint32_t, static_cast<size_t>(QueueFamily::count)>;

struct SwapchainManager {
public:
  SwapchainManager(vk::raii::PhysicalDevice const& physicalDevice,
                   vk::raii::Device const& device,
                   vk::raii::SurfaceKHR const& surface,
                   GLFWwindow* window,
                   QueueFamilyIndexMap const& queueFamilyIndices)
      : physicalDevice(physicalDevice),
        device(device),
        surface(surface),
        queueFamilyIndices(queueFamilyIndices),
        window(window) {
    querySurfaceDetails();
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
  void querySurfaceDetails();

  constexpr vk::SurfaceFormatKHR const& getSurfaceFormat() const {
    return surfaceFormat;
  }
  constexpr vk::Extent2D const& getExtent() const { return swapchainExtent; }
  constexpr vk::SwapchainKHR const& getSwapchain() const { return *swapchain; }
  constexpr std::vector<vk::raii::ImageView> const& getImageViews() const {
    return imageViews;
  }

  static std::optional<vk::PresentModeKHR> choosePresentMode(
      const std::vector<vk::PresentModeKHR>& availablePresentModes);
  static vk::Extent2D computeSwapchainExtent(
      const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
      GLFWwindow* window);

private:
  vk::raii::PhysicalDevice const& physicalDevice;
  vk::raii::Device const& device;
  vk::raii::SurfaceKHR const& surface;
  QueueFamilyIndexMap const& queueFamilyIndices;
  GLFWwindow* window;

  vk::SurfaceFormatKHR surfaceFormat;
  vk::SurfaceCapabilitiesKHR surfaceCapabilities;
  vk::PresentModeKHR presentMode;
  vk::Extent2D swapchainExtent;

  vk::raii::SwapchainKHR swapchain = nullptr;
  std::vector<vk::Image> images;
  std::vector<vk::raii::ImageView> imageViews;
};
