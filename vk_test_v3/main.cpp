// **INCOMPLETE PROJECT**

// I should probably be using SDL, but I wanted to
// try my best to power through the tutorial this time.

// I am also still in the process of modularizing this code.
// `util` and `SwapchainManager` are nicer.

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

constexpr std::array validationLayers{
    "VK_LAYER_KHRONOS_validation",
};

constexpr std::array deviceExtensions{
    vk::KHRSwapchainExtensionName,
};

constexpr char const* AppName = "Hello Triangle";

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
  SDLContext gContext;
  vk::raii::Context context;

  auto window = SDLWindow(gContext, AppName, 500, 500);

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

  // TODO: Configure createInfo.
  auto instance =
      createInstance(context, AppName, validationLayers, &debugCreateInfo);

  auto debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo);

  auto surface = SDLSurface(window, instance);

  // TODO: Pick desired `PhysicalDevice` using some metric.
  auto [physicalDevice, queueFamilyIndices] =
      pickPhysicalDevice(instance, surface, deviceExtensions);

  // TODO: Configure createInfo.
  auto device =
      createDevice(physicalDevice, queueFamilyIndices, deviceExtensions);

  auto graphicsQueue = device.getQueue(queueFamilyIndices.graphicsFamily, 0);
  auto presentQueue = device.getQueue(queueFamilyIndices.presentFamily, 0);

  SwapchainManager swapchainManager{
      device, physicalDevice, surface, window, queueFamilyIndices};

  auto renderPass = createRenderPass(device, swapchainManager.surfaceFormat);

  auto vertexShaderCode = readFile(executableDirectory / "shader.vert.spv");
  auto vertexShader = createShaderModule(device, vertexShaderCode);

  auto fragmentShaderCode = readFile(executableDirectory / "shader.frag.spv");
  auto fragmentShader = createShaderModule(device, fragmentShaderCode);

  vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo;
  vertexShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
  vertexShaderStageCreateInfo.module = vertexShader;
  vertexShaderStageCreateInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo;
  fragmentShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
  fragmentShaderStageCreateInfo.module = fragmentShader;
  fragmentShaderStageCreateInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo shaderStages[] = {
      vertexShaderStageCreateInfo,
      fragmentShaderStageCreateInfo,
  };

  auto graphicsPipeline = createGraphicsPipeline(
      device, swapchainManager.extent, renderPass, shaderStages);

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

  vk::CommandPoolCreateInfo commandPoolCreateInfo;
  commandPoolCreateInfo.flags =
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

  auto commandPool = device.createCommandPool(commandPoolCreateInfo);

  constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

  auto commandBuffers =
      device.allocateCommandBuffers(commandBufferAllocateInfo);

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

    [[maybe_unused]]
    auto _temp0 = device.waitForFences(*inFlightFences[currentFrame],
                                       true,
                                       std::numeric_limits<uint64_t>::max());
    device.resetFences(*inFlightFences[currentFrame]);

    vk::AcquireNextImageInfoKHR acquireInfo;
    acquireInfo.swapchain = swapchainManager.swapchain;
    acquireInfo.timeout = std::numeric_limits<uint64_t>::max();
    acquireInfo.semaphore = imageAvailableSemaphores[currentFrame];
    acquireInfo.deviceMask = 0b1;

    auto imageIndex = device.acquireNextImage2KHR(acquireInfo).second;

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
  }

  device.waitIdle(); // Validation compliance.
}
