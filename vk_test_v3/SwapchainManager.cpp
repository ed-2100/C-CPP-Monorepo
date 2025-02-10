#include "SwapchainManager.hpp"
#include <SDL3/SDL_vulkan.h>

#include <ranges>
#include <unordered_map>
#include <unordered_set>

void SwapchainManager::createSwapchain(vk::SwapchainKHR oldSwapchain) {
  vk::SwapchainCreateInfoKHR createInfo;
  createInfo.surface = surface;
  createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
  createInfo.preTransform = surfaceCapabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode = presentMode;
  createInfo.oldSwapchain = oldSwapchain;
  createInfo.clipped = true;

  // FIXME: Not sure if this covers all the cases.
  auto uniqueFamilyIndices = queueFamilyIndices.families |
                             std::ranges::to<std::unordered_set>() |
                             std::ranges::to<std::vector>();
  if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
  }
  createInfo.setQueueFamilyIndices(uniqueFamilyIndices);

  swapchain = device.createSwapchainKHR(createInfo);
  images = swapchain.getImages();
}

void SwapchainManager::recreateSwapchain() {
  device.waitIdle();
  queryDetails();
  auto oldSwapchain = std::move(swapchain);
  createSwapchain(oldSwapchain);
  createImageViews();
}

void SwapchainManager::createImageViews() {
  imageViews.clear();
  imageViews.reserve(images.size());
  for (auto const& image : images) {
    vk::ImageViewCreateInfo createInfo;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = surfaceFormat.format;
    createInfo.components = vk::ComponentSwizzle{};
    createInfo.subresourceRange =
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    createInfo.image = image;

    imageViews.push_back(device.createImageView(createInfo));
  }
}

void SwapchainManager::queryDetails() {
  surfaceFormat = [&]() {
    for (auto const& availableFormat :
         physicalDevice.getSurfaceFormatsKHR(surface)) {
      if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
          availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        return availableFormat;
      }
    }
    throw std::runtime_error(
        "The surface does not support the desired format!");
  }();

  auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
  if (presentModes.empty()) throw std::runtime_error("No present modes found!");

  presentMode = choosePresentMode(presentModes);

  surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
  extent = computeSwapchainExtent(surfaceCapabilities, window);
}

vk::Extent2D SwapchainManager::computeSwapchainExtent(
    vk::SurfaceCapabilitiesKHR const& surfaceCapabilities,
    Window const& window) {
  if (surfaceCapabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return surfaceCapabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = window.queryExtent();

    actualExtent.width = std::clamp(actualExtent.width,
                                    surfaceCapabilities.minImageExtent.width,
                                    surfaceCapabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
                                     surfaceCapabilities.minImageExtent.height,
                                     surfaceCapabilities.maxImageExtent.height);

    return actualExtent;
  }
}

vk::PresentModeKHR SwapchainManager::choosePresentMode(
    std::vector<vk::PresentModeKHR> const& availablePresentModes) {
  const std::unordered_map<vk::PresentModeKHR, uint32_t> presentModePreference{
      {vk::PresentModeKHR::eMailbox, 0},
      {vk::PresentModeKHR::eFifo, 1},
  };

  vk::PresentModeKHR presentMode;

  uint32_t currentRating = std::numeric_limits<uint32_t>::max();
  for (const auto& availablePresentMode : availablePresentModes) {
    auto candidateKVPairIter = presentModePreference.find(availablePresentMode);
    if (candidateKVPairIter == presentModePreference.cend()) continue;

    uint32_t candidateRating = candidateKVPairIter->second;
    if (candidateRating < currentRating) {
      presentMode = availablePresentMode;
      currentRating = candidateRating;
    }
  }

  if (currentRating == std::numeric_limits<uint32_t>::max()) {
    throw std::runtime_error("Failed to choose a desired present mode!");
  }

  return presentMode;
}
