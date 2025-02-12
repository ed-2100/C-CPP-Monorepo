#pragma once

#include <filesystem>
#include <thread>
#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include "HelperFunctions.hpp"

struct Renderer;

struct SwapchainManager {
public:
  SwapchainManager() = delete;
  SwapchainManager(std::nullptr_t) {}
  SwapchainManager(Renderer& renderer) : renderer(&renderer) {
    createSwapchain();
  }

  SwapchainManager(const SwapchainManager&) = delete;
  SwapchainManager& operator=(const SwapchainManager&) = delete;

  SwapchainManager(SwapchainManager&& rhs) = default;
  SwapchainManager& operator=(SwapchainManager&& other) = default;

  void createSwapchain(vk::SwapchainKHR oldSwapchain = nullptr);
  void recreateSwapchain();

  Renderer* renderer;

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
};

const std::array validationLayers1{
    "VK_LAYER_KHRONOS_validation",
};

const std::array deviceExtensions1{
    vk::KHRSwapchainExtensionName,
};

char const* const appName1 = "Hello Triangle";

struct Renderer {
  std::shared_ptr<SDLContext> sdlContext = nullptr;
  vk::raii::Context vkContext;
  SDLWindow window = nullptr;
  vk::raii::Instance instance = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  SDLSurface surface = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  QueueFamilyIndexMap queueFamilyIndices;
  vk::raii::Device device = nullptr;
  vk::raii::Queue graphicsQueue = nullptr, presentQueue = nullptr;
  SwapchainManager swapchainManager = nullptr;
  vk::raii::RenderPass renderPass = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;
  std::vector<vk::raii::Framebuffer> framebuffers;
  vk::raii::CommandPool commandPool = nullptr;
  std::vector<vk::raii::CommandBuffer> commandBuffers;
  std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> inFlightFences;
  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  Renderer() = delete;
  Renderer(std::filesystem::path const& exeDir) {
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

    device =
        createDevice(physicalDevice, queueFamilyIndices, deviceExtensions1);

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
      commandPoolCreateInfo.queueFamilyIndex =
          queueFamilyIndices.graphicsFamily;

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

  void run() {
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

      commandBuffers[currentFrame].beginRenderPass(
          renderPassBeginInfo, vk::SubpassContents::eInline);
      commandBuffers[currentFrame].bindPipeline(
          vk::PipelineBindPoint::eGraphics, graphicsPipeline);
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
};
