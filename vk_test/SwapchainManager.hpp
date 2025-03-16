#pragma once

#include "Forward.hpp"

struct SwapchainManager {
public:
    SwapchainManager() = delete;

    SwapchainManager(std::nullptr_t) {}

    SwapchainManager(Application& renderer) : renderer(&renderer) { createSwapchain(); }

    SwapchainManager(const SwapchainManager&) = delete;
    SwapchainManager& operator=(const SwapchainManager&) = delete;

    SwapchainManager(SwapchainManager&& rhs) = default;
    SwapchainManager& operator=(SwapchainManager&& other) = default;

    void createSwapchain(vk::SwapchainKHR oldSwapchain = nullptr);
    void recreateSwapchain();

    Application* renderer;

    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D extent;

    vk::raii::SwapchainKHR swapchain = nullptr;

    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;

private:
    vk::PresentModeKHR choosePresentMode() const;
    vk::SurfaceFormatKHR chooseSurfaceFormat() const;
    vk::Extent2D chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) const;
};
