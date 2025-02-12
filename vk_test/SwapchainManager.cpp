#include "SwapchainManager.hpp"

#include <SDL3/SDL_vulkan.h>

#include <ranges>
#include <thread>
#include <unordered_map>
#include <unordered_set>

vk::SurfaceFormatKHR SwapchainManager::chooseSurfaceFormat() const {
  for (auto const& availableFormat :
       renderer->physicalDevice.getSurfaceFormatsKHR(renderer->surface)) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }
  throw std::runtime_error("The surface does not support the desired format!");
}

void SwapchainManager::createSwapchain(vk::SwapchainKHR oldSwapchain) {
  surfaceFormat = chooseSurfaceFormat();
  auto presentMode = choosePresentMode();
  auto surfaceCapabilities =
      renderer->physicalDevice.getSurfaceCapabilitiesKHR(renderer->surface);
  extent = chooseSwapchainExtent(surfaceCapabilities);

  vk::SwapchainCreateInfoKHR createInfo;
  createInfo.surface = renderer->surface;
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

  auto uniqueFamilyIndices = renderer->queueFamilyIndices.families |
                             std::ranges::to<std::unordered_set>() |
                             std::ranges::to<std::vector>();
  if (renderer->queueFamilyIndices.graphicsFamily !=
      renderer->queueFamilyIndices.presentFamily) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
  }
  createInfo.setQueueFamilyIndices(uniqueFamilyIndices);

  swapchain = renderer->device.createSwapchainKHR(createInfo);

  images = swapchain.getImages();

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

    imageViews.push_back(renderer->device.createImageView(createInfo));
  }
}

void SwapchainManager::recreateSwapchain() {
  renderer->device.waitIdle();
  auto oldSwapchain = std::move(swapchain);
  createSwapchain(oldSwapchain);
}

vk::Extent2D SwapchainManager::chooseSwapchainExtent(
    vk::SurfaceCapabilitiesKHR const& surfaceCapabilities) const {
  if (surfaceCapabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return surfaceCapabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = renderer->window.queryExtent();

    actualExtent.width = std::clamp(actualExtent.width,
                                    surfaceCapabilities.minImageExtent.width,
                                    surfaceCapabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
                                     surfaceCapabilities.minImageExtent.height,
                                     surfaceCapabilities.maxImageExtent.height);

    return actualExtent;
  }
}

vk::PresentModeKHR SwapchainManager::choosePresentMode() const {
  auto presentModes =
      renderer->physicalDevice.getSurfacePresentModesKHR(renderer->surface);
  if (presentModes.empty()) throw std::runtime_error("No present modes found!");

  const std::unordered_map<vk::PresentModeKHR, uint32_t> presentModePreference{
      {vk::PresentModeKHR::eMailbox, 0},
      {vk::PresentModeKHR::eFifo, 1},
  };

  vk::PresentModeKHR presentMode;

  uint32_t currentRating = std::numeric_limits<uint32_t>::max();
  for (const auto& availablePresentMode : presentModes) {
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

Renderer::Renderer(std::filesystem::path const& exeDir) {
  sdlContext = SDLContext::getInstance();

  window = SDLWindow(appName1, 500, 500);

  vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  debugCreateInfo.messageSeverity =
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
  debugCreateInfo.messageType =
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  debugCreateInfo.pfnUserCallback = debugCallback;

  instance = createInstance(
      *sdlContext, vkContext, appName1, validationLayers1, &debugCreateInfo);

  debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo);

  surface = SDLSurface(window, instance);

  std::tie(physicalDevice, queueFamilyIndices) =
      pickPhysicalDevice(instance, surface, deviceExtensions1);

  device = createDevice(physicalDevice, queueFamilyIndices, deviceExtensions1);

  graphicsQueue = device.getQueue(queueFamilyIndices.graphicsFamily, 0);
  presentQueue = device.getQueue(queueFamilyIndices.presentFamily, 0);

  swapchainManager = SwapchainManager{*this};

  renderPass = createRenderPass(device, swapchainManager.surfaceFormat);

  graphicsPipeline = createGraphicsPipeline(
      device, swapchainManager.extent, renderPass, exeDir);

  framebuffers.reserve(swapchainManager.imageViews.size());
  for (auto const& imageView : swapchainManager.imageViews) {
    vk::FramebufferCreateInfo createInfo;
    createInfo.renderPass = renderPass;
    createInfo.width = swapchainManager.extent.width;
    createInfo.height = swapchainManager.extent.height;
    createInfo.layers = 1;
    createInfo.setAttachments(*imageView);

    framebuffers.push_back(device.createFramebuffer(createInfo));
  }

  commandPool = [this]() {
    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.flags =
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    return device.createCommandPool(commandPoolCreateInfo);
  }();

  commandBuffers = [this]() {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    return device.allocateCommandBuffers(commandBufferAllocateInfo);
  }();

  imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::SemaphoreCreateInfo semaphoreCreateInfo;

    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

    imageAvailableSemaphores.push_back(
        device.createSemaphore(semaphoreCreateInfo));
    renderFinishedSemaphores.push_back(
        device.createSemaphore(semaphoreCreateInfo));
    inFlightFences.push_back(device.createFence(fenceCreateInfo));
  }
}
void Renderer::run() {
  bool done = false;
  for (uint32_t currentFrame = 0; !done;
       currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT) {
    [[maybe_unused]]
    auto _temp0 = device.waitForFences(*inFlightFences[currentFrame],
                                       true,
                                       std::numeric_limits<uint64_t>::max());
    device.resetFences(*inFlightFences[currentFrame]);

    auto imageIndex = [&]() {
      vk::AcquireNextImageInfoKHR acquireInfo;
      acquireInfo.swapchain = swapchainManager.swapchain;
      acquireInfo.timeout = std::numeric_limits<uint64_t>::max();
      acquireInfo.semaphore = imageAvailableSemaphores[currentFrame];
      acquireInfo.deviceMask = 0b1;

      return device.acquireNextImage2KHR(acquireInfo).second;
    }();

    commandBuffers[currentFrame].reset();

    commandBuffers[currentFrame].begin(vk::CommandBufferBeginInfo{});

    vk::ClearValue clearColor;
    clearColor.setColor({0, 0, 0, 0});

    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffers[imageIndex];
    renderPassBeginInfo.renderArea =
        vk::Rect2D{{0, 0}, swapchainManager.extent};
    renderPassBeginInfo.setClearValues(clearColor);

    commandBuffers[currentFrame].beginRenderPass(renderPassBeginInfo,
                                                 vk::SubpassContents::eInline);
    commandBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics,
                                              graphicsPipeline);
    commandBuffers[currentFrame].draw(3, 1, 0, 0);
    commandBuffers[currentFrame].endRenderPass();
    commandBuffers[currentFrame].end();

    std::array<vk::PipelineStageFlags, 1> waitStageMasks{
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submitInfo;
    submitInfo.setWaitSemaphores(*imageAvailableSemaphores[currentFrame]);
    submitInfo.setWaitDstStageMask(waitStageMasks);
    submitInfo.setCommandBuffers(*commandBuffers[currentFrame]);
    submitInfo.setSignalSemaphores(*renderFinishedSemaphores[currentFrame]);

    graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(*renderFinishedSemaphores[currentFrame]);
    presentInfo.setSwapchains(*swapchainManager.swapchain);
    presentInfo.setImageIndices(imageIndex);

    [[maybe_unused]] auto _temp1 = presentQueue.presentKHR(presentInfo);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      } else if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          done = true;
        }
      }
    }
  }

  device.waitIdle(); // Validation compliance.
}
