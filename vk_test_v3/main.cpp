// **INCOMPLETE PROJECT**

// I should probably be using SDL, but I wanted to
// try my best to power through the tutorial this time.

// I am also still in the process of modularizing this code.
// `util` and `SwapchainManager` are nicer.

#include "SwapchainManager.hpp"
#include "util.hpp"

#include <SDL3/SDL_vulkan.h>
#include <thread>
#include <vulkan/vulkan_raii.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
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

static std::vector<char const*> getInstanceExtensions() {
  decltype(getInstanceExtensions()) extensions;
  uint32_t glfwExtensionCount;
  char const* const* glfwExtensions =
      SDL_Vulkan_GetInstanceExtensions(&glfwExtensionCount);
  if (nullptr == glfwExtensions) {
    throw std::runtime_error("Failed to get glfw's required extensions!");
  }
  for (auto extension : std::span(glfwExtensions, glfwExtensionCount)) {
    extensions.push_back(extension);
  }
  extensions.push_back("VK_EXT_debug_utils");
  extensions.push_back("VK_KHR_portability_enumeration");
  return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
              VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
              VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
              void* /*pUserData*/) {
  std::cerr << "Validation layer message: " << pCallbackData->pMessage << '\n';

  return VK_FALSE;
}

static vk::raii::Instance createInstance(vk::raii::Context const& context,
                                         void* pNext) {
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName = AppName;
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
  appInfo.pEngineName = "None";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  auto instanceExtensions = getInstanceExtensions();

  vk::InstanceCreateInfo createInfo;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
  createInfo.pNext = pNext;
  createInfo.setPEnabledLayerNames(validationLayers);
  createInfo.setPEnabledExtensionNames(instanceExtensions);

  return context.createInstance(createInfo);
}

static vk::raii::RenderPass createRenderPass(
    vk::raii::Device const& device, vk::SurfaceFormatKHR const& surfaceFormat) {
  vk::AttachmentDescription colorAttachment;
  colorAttachment.format = surfaceFormat.format;
  colorAttachment.samples = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef{
      0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpass;
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.setColorAttachments(colorAttachmentRef);

  vk::SubpassDependency subpassDependency;
  subpassDependency.srcSubpass = vk::SubpassExternal;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;
  subpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
  subpassDependency.dstStageMask =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;
  subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo renderPassCreateInfo;
  renderPassCreateInfo.setAttachments(colorAttachment);
  renderPassCreateInfo.setSubpasses(subpass);
  renderPassCreateInfo.setDependencies(subpassDependency);

  return device.createRenderPass(renderPassCreateInfo);
}

static vk::raii::Pipeline createGraphicsPipeline(
    vk::raii::Device const& device,
    vk::Extent2D const& swapchainExtent,
    vk::raii::RenderPass const& renderPass,
    vk::ArrayProxyNoTemporaries<vk::PipelineShaderStageCreateInfo> const&
        shaderStages) {
  vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
  inputAssemblyStateCreateInfo.setTopology(
      vk::PrimitiveTopology::eTriangleList);
  inputAssemblyStateCreateInfo.setPrimitiveRestartEnable(false);

  vk::Viewport viewport;
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = swapchainExtent.width;
  viewport.height = swapchainExtent.height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0;

  vk::Rect2D scissor{{0, 0}, swapchainExtent};

  vk::PipelineViewportStateCreateInfo viewportStateCreateInfo;
  viewportStateCreateInfo.setViewports(viewport);
  viewportStateCreateInfo.setScissors(scissor);

  vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
  rasterizationStateCreateInfo.depthClampEnable = false;
  rasterizationStateCreateInfo.rasterizerDiscardEnable = false;
  rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
  rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
  rasterizationStateCreateInfo.frontFace = vk::FrontFace::eClockwise;
  rasterizationStateCreateInfo.depthBiasEnable = false;
  rasterizationStateCreateInfo.lineWidth = 1.0; // Validation compliance.

  vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
  multisampleStateCreateInfo.sampleShadingEnable = false;

  vk::PipelineColorBlendAttachmentState colorBlending;
  colorBlending.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlending.blendEnable = false;
  colorBlending.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlending.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlending.colorBlendOp = vk::BlendOp::eAdd;
  colorBlending.srcAlphaBlendFactor = vk::BlendFactor::eOne;
  colorBlending.dstAlphaBlendFactor = vk::BlendFactor::eZero;
  colorBlending.alphaBlendOp = vk::BlendOp::eAdd;

  vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
  colorBlendStateCreateInfo.logicOpEnable = false;
  colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
  colorBlendStateCreateInfo.setAttachments(colorBlending);

  auto pipelineLayout =
      device.createPipelineLayout(vk::PipelineLayoutCreateInfo{});

  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
  graphicsPipelineCreateInfo.setStages(shaderStages);
  graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
  graphicsPipelineCreateInfo.pInputAssemblyState =
      &inputAssemblyStateCreateInfo;
  graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  graphicsPipelineCreateInfo.pRasterizationState =
      &rasterizationStateCreateInfo;
  graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
  graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
  graphicsPipelineCreateInfo.layout = pipelineLayout;
  graphicsPipelineCreateInfo.renderPass = renderPass;
  graphicsPipelineCreateInfo.subpass = 0;

  return device.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
}

static auto pickPhysicalDevice(vk::raii::Instance const& instance,
                               vk::raii::SurfaceKHR const& surface) {
  auto physicalDevices = instance.enumeratePhysicalDevices();

  for (auto& physicalDevice : physicalDevices) {
    auto vFamilyProperties = physicalDevice.getQueueFamilyProperties();

    std::optional<uint32_t> graphicsFamily;
    for (const auto [i, familyProperties] :
         std::views::enumerate(vFamilyProperties)) {
      if (familyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
        graphicsFamily = i;
        break;
      }
    }
    if (!graphicsFamily) continue;

    std::optional<uint32_t> presentFamily;
    for (const auto [i, familyProperties] :
         std::views::enumerate(vFamilyProperties)) {
      if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
        presentFamily = i;
        break;
      }
    }
    if (!presentFamily) continue;

    auto vExtensionProperties =
        physicalDevice.enumerateDeviceExtensionProperties();

    std::set<std::string> extensionChecklist{deviceExtensions.cbegin(),
                                             deviceExtensions.cend()};
    for (auto const& extensionProperties : vExtensionProperties) {
      extensionChecklist.erase(extensionProperties.extensionName);
    }
    if (!extensionChecklist.empty()) continue;

    return std::make_tuple(std::move(physicalDevice),
                           graphicsFamily.value(),
                           presentFamily.value());
  }

  throw std::runtime_error("No suitable GPU.");
}

static vk::raii::Device createDevice(
    vk::raii::PhysicalDevice const& physicalDevice,
    uint32_t graphicsFamily,
    uint32_t presentFamily) {
  vk::PhysicalDeviceFeatures deviceFeatures;

  std::set<uint32_t> uniqueQueueFamilyIndices = {graphicsFamily, presentFamily};

  float queuePriority = 1.0;
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  queueCreateInfos.reserve(uniqueQueueFamilyIndices.size());
  for (auto uniqueQueueFamilyIndex : uniqueQueueFamilyIndices) {
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = uniqueQueueFamilyIndex;
    queueCreateInfo.setQueuePriorities(queuePriority);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::DeviceCreateInfo createInfo;
  createInfo.setQueueCreateInfos(queueCreateInfos);
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.setPEnabledExtensionNames(deviceExtensions);

  return physicalDevice.createDevice(createInfo);
}

static std::vector<uint32_t> readFile(std::filesystem::path const& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) throw std::runtime_error("Failed to open file!");

  size_t size = file.tellg();
  if (size % sizeof(uint32_t) != 0) // Will make this more robust as necessary.
    throw std::runtime_error("File size is not a multiple of uint32_t!");

  file.seekg(0);

  std::vector<uint32_t> fileContents;
  fileContents.resize(size / 4);
  file.read(reinterpret_cast<char*>(fileContents.data()), size);
  if (!file) throw std::runtime_error("Error reading file!");

  return fileContents;
}

static vk::raii::ShaderModule createShaderModule(
    vk::raii::Device const& device, std::vector<uint32_t> const& code) {
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.setCode(code);
  return device.createShaderModule(createInfo);
}

void run(std::filesystem::path const& executableDirectory) {
  glfwContext gContext;
  vk::raii::Context context;

  auto window = gContext.createWindow(AppName, vk::Extent2D(500, 500));

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
  auto instance = createInstance(context, &debugCreateInfo);

  auto debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo);

  auto surface = window.createSurface(instance);

  // TODO: Pick desired `PhysicalDevice` using some metric.
  auto [physicalDevice, graphicsFamily, presentFamily] =
      pickPhysicalDevice(instance, surface);

  // TODO: Configure createInfo.
  auto device = createDevice(physicalDevice, graphicsFamily, presentFamily);

  auto graphicsQueue = device.getQueue(graphicsFamily, 0);
  auto presentQueue = device.getQueue(presentFamily, 0);

  QueueFamilyIndexMap queueFamilyIndices;
  queueFamilyIndices[QueueFamily::graphicsFamily] = graphicsFamily;
  queueFamilyIndices[QueueFamily::presentFamily] = presentFamily;

  SwapchainManager swapchainManager{
      physicalDevice, device, surface, window.getHandle(), queueFamilyIndices};

  auto renderPass =
      createRenderPass(device, swapchainManager.getSurfaceFormat());

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
      device, swapchainManager.getExtent(), renderPass, shaderStages);

  std::vector<vk::raii::Framebuffer> framebuffers;
  framebuffers.reserve(swapchainManager.getImageViews().size());
  for (auto const& imageView : swapchainManager.getImageViews()) {
    vk::FramebufferCreateInfo createInfo;
    createInfo.renderPass = renderPass;
    createInfo.width = swapchainManager.getExtent().width;
    createInfo.height = swapchainManager.getExtent().height;
    createInfo.layers = 1;
    createInfo.setAttachments(*imageView);

    framebuffers.push_back(device.createFramebuffer(createInfo));
  }

  vk::CommandPoolCreateInfo commandPoolCreateInfo;
  commandPoolCreateInfo.flags =
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  commandPoolCreateInfo.queueFamilyIndex = graphicsFamily;

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
    acquireInfo.swapchain = swapchainManager.getSwapchain();
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
        vk::Rect2D{{0, 0}, swapchainManager.getExtent()};
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
    presentInfo.setSwapchains(swapchainManager.getSwapchain());
    presentInfo.setImageIndices(imageIndex);

    [[maybe_unused]] auto _temp1 = presentQueue.presentKHR(presentInfo);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  device.waitIdle(); // Validation compliance.
}
