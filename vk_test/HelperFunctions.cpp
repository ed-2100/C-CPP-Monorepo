#include "HelperFunctions.hpp"

#include <fstream>
#include <iostream>
#include <ranges>
#include <unordered_set>

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
              VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
              VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
              void* /*pUserData*/) {
  std::cerr << "Validation layer message: " << pCallbackData->pMessage << '\n';

  return VK_FALSE;
}

vk::raii::Instance createInstance(SDLContext const& sdlContext,
                                  vk::raii::Context const& context,
                                  char const* appName,
                                  std::span<char const* const> validationLayers,
                                  void* pNext) {
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName = appName;
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
  appInfo.pEngineName = "None";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  auto sdlExtensions = sdlContext.getInstanceExtensions();
  std::vector<char const*> instanceExtensions(sdlExtensions.cbegin(),
                                              sdlExtensions.cend());
  instanceExtensions.push_back("VK_EXT_debug_utils");
  instanceExtensions.push_back("VK_KHR_portability_enumeration");

  vk::InstanceCreateInfo createInfo;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
  createInfo.pNext = pNext;
  createInfo.setPEnabledLayerNames(validationLayers);
  createInfo.setPEnabledExtensionNames(instanceExtensions);

  return context.createInstance(createInfo);
}

vk::raii::RenderPass createRenderPass(
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

vk::raii::Pipeline createGraphicsPipeline(
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

std::tuple<vk::raii::PhysicalDevice, QueueFamilyIndexMap> pickPhysicalDevice(
    vk::raii::Instance const& instance,
    vk::SurfaceKHR const& surface,
    std::span<char const* const> deviceExtensions) {
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

    std::unordered_set<std::string> extensionChecklist{
        deviceExtensions.cbegin(), deviceExtensions.cend()};
    for (auto const& extensionProperties : vExtensionProperties) {
      extensionChecklist.erase(extensionProperties.extensionName);
    }
    if (!extensionChecklist.empty()) continue;

    return std::make_tuple(
        std::move(physicalDevice),
        QueueFamilyIndexMap{.graphicsFamily = graphicsFamily.value(),
                            .presentFamily = presentFamily.value()});
  }

  throw std::runtime_error("No suitable GPU.");
}

vk::raii::Device createDevice(vk::raii::PhysicalDevice const& physicalDevice,
                              QueueFamilyIndexMap const& queueFamilyIndices,
                              std::span<char const* const> deviceExtensions) {
  vk::PhysicalDeviceFeatures deviceFeatures;

  auto uniqueQueueFamilyIndices =
      queueFamilyIndices.families | std::ranges::to<std::unordered_set>();

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

std::vector<uint32_t> readFile(std::filesystem::path const& filename) {
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

vk::raii::ShaderModule createShaderModule(vk::raii::Device const& device,
                                          std::vector<uint32_t> const& code) {
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.setCode(code);
  return device.createShaderModule(createInfo);
}
