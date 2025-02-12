// **INCOMPLETE PROJECT**

// I am still in the process of modularizing this code.
// `SDLUtils.{hpp,cpp}` and `SwapchainManager.{hpp,cpp}` are nicer.

#include "HelperFunctions.hpp"
#include "SDLUtils.hpp"
#include "SwapchainManager.hpp"

#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_vulkan.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <thread>
#include <vector>

const std::array validationLayers{
    "VK_LAYER_KHRONOS_validation",
};

const std::array deviceExtensions{
    vk::KHRSwapchainExtensionName,
};

char const* const appName = "Hello Triangle";

void run(const std::filesystem::path&);

int main(int /*argc*/, char const* const* argv) {
  try {
    run(std::filesystem::canonical(std::filesystem::path(argv[0]))
            .parent_path());
  } catch (const std::exception& e) {
    std::cout << typeid(e).name() << ": " << e.what() << std::endl;
  } catch (...) {
    std::cout << "Unknown exception.";
  }
}

void run(std::filesystem::path const& executableDirectory) {
  auto sdlContext = SDLContext::getInstance();

  vk::raii::Context context;

  auto window = SDLWindow(appName, 500, 500);

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

  auto instance = createInstance(
      *sdlContext, context, appName, validationLayers, &debugCreateInfo);

  auto debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo);

  auto surface = SDLSurface(window, instance);

  auto [physicalDevice, queueFamilyIndices] =
      pickPhysicalDevice(instance, surface, deviceExtensions);

  auto device =
      createDevice(physicalDevice, queueFamilyIndices, deviceExtensions);

  auto graphicsQueue = device.getQueue(queueFamilyIndices.graphicsFamily, 0);
  auto presentQueue = device.getQueue(queueFamilyIndices.presentFamily, 0);

  SwapchainManager swapchainManager{
      device, physicalDevice, surface, window, queueFamilyIndices};

  auto renderPass = createRenderPass(device, swapchainManager.surfaceFormat);

  auto graphicsPipeline = createGraphicsPipeline(
      device, swapchainManager.extent, renderPass, executableDirectory);

  std::vector<vk::raii::Framebuffer> framebuffers;
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

  auto commandPool = [&device, &queueFamilyIndices]() {
    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.flags =
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    return device.createCommandPool(commandPoolCreateInfo);
  }();

  constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  auto commandBuffers = [&device, &commandPool]() {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    return device.allocateCommandBuffers(commandBufferAllocateInfo);
  }();

  std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> inFlightFences;
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
