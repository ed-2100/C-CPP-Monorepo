#include "Program.hpp"

#include <SDL3/SDL_vulkan.h>

#include <fstream>
#include <iostream>
#include <ranges>
#include <thread>
#include <unordered_set>

const std::array validationLayers1 {
    "VK_LAYER_KHRONOS_validation",
};

const std::array deviceExtensions1 {
    vk::KHRSwapchainExtensionName,
};

char const* const appName1 = "Hello Triangle";

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
    vk::DebugUtilsMessageTypeFlagsEXT /*messageType*/,
    vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    void* /*pUserData*/
) {
    std::cerr << "Validation layer message: " << pCallbackData->pMessage
              << '\n';

    return VK_FALSE;
}

Application::Application(std::filesystem::path const& exeDir) {
    sdlContext = SDLContext::getInstance();

    window = SDLWindow(appName1, 500, 500);

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    debugCreateInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    debugCreateInfo.pfnUserCallback = debugCallback;

    instance = createInstance(*sdlContext, vkContext, &debugCreateInfo);

    debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo);

    surface = SDLSurface(window, instance);

    std::tie(physicalDevice, queueFamilyIndices) =
        pickPhysicalDevice(instance, surface, deviceExtensions1);

    device =
        createDevice(physicalDevice, queueFamilyIndices, deviceExtensions1);

    graphicsQueue = device.getQueue(queueFamilyIndices.graphicsFamily, 0);
    presentQueue = device.getQueue(queueFamilyIndices.presentFamily, 0);

    swapchainManager = SwapchainManager {*this};

    renderPass = createRenderPass(device, swapchainManager.surfaceFormat);

    graphicsPipeline = createGraphicsPipeline(
        device,
        swapchainManager.extent,
        renderPass,
        exeDir
    );

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
            device.createSemaphore(semaphoreCreateInfo)
        );
        renderFinishedSemaphores.push_back(
            device.createSemaphore(semaphoreCreateInfo)
        );
        inFlightFences.push_back(device.createFence(fenceCreateInfo));
    }
}

void Application::run() {
    bool done = false;
    for (uint32_t currentFrame = 0; !done;
         currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT) {
        [[maybe_unused]]
        auto _temp0 = device.waitForFences(
            *inFlightFences[currentFrame],
            true,
            std::numeric_limits<uint64_t>::max()
        );
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

        commandBuffers[currentFrame].begin(vk::CommandBufferBeginInfo {});

        vk::ClearValue clearColor;
        clearColor.setColor({0, 0, 0, 0});

        vk::RenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffers[imageIndex];
        renderPassBeginInfo.renderArea =
            vk::Rect2D {{0, 0}, swapchainManager.extent};
        renderPassBeginInfo.setClearValues(clearColor);

        commandBuffers[currentFrame].beginRenderPass(
            renderPassBeginInfo,
            vk::SubpassContents::eInline
        );
        commandBuffers[currentFrame].bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            graphicsPipeline
        );
        commandBuffers[currentFrame].draw(3, 1, 0, 0);
        commandBuffers[currentFrame].endRenderPass();
        commandBuffers[currentFrame].end();

        std::array<vk::PipelineStageFlags, 1> waitStageMasks {
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        };
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

vk::raii::Instance Application::createInstance(
    SDLContext const& sdlContext,
    vk::raii::Context& vkContext,
    void* pNext
) {
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = appName1;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "None";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    auto sdlExtensions = sdlContext.getInstanceExtensions();
    std::vector<char const*> instanceExtensions(
        sdlExtensions.cbegin(),
        sdlExtensions.cend()
    );
    instanceExtensions.push_back("VK_EXT_debug_utils");
    instanceExtensions.push_back("VK_KHR_portability_enumeration");

    vk::InstanceCreateInfo createInfo {};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.pNext = pNext;
    createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    createInfo.setPEnabledLayerNames(validationLayers1);
    createInfo.setPEnabledExtensionNames(instanceExtensions);

    return vkContext.createInstance(createInfo);
}

vk::raii::RenderPass Application::createRenderPass(
    vk::raii::Device const& device,
    vk::SurfaceFormatKHR const& surfaceFormat
) {
    vk::AttachmentDescription colorAttachment;
    colorAttachment.format = surfaceFormat.format;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef {
        0,
        vk::ImageLayout::eColorAttachmentOptimal
    };

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

vk::raii::Pipeline Application::createGraphicsPipeline(
    vk::raii::Device const& device,
    vk::Extent2D const& swapchainExtent,
    vk::raii::RenderPass const& renderPass,
    std::filesystem::path const& exeDir
) {
    auto vertexShaderCode = readFile(exeDir / "shader.vert.spv");
    auto vertexShader = createShaderModule(device, vertexShaderCode);

    auto fragmentShaderCode = readFile(exeDir / "shader.frag.spv");
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

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    inputAssemblyStateCreateInfo.setTopology(
        vk::PrimitiveTopology::eTriangleList
    );
    inputAssemblyStateCreateInfo.setPrimitiveRestartEnable(false);

    vk::Viewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = swapchainExtent.width;
    viewport.height = swapchainExtent.height;
    viewport.minDepth = 0.0;
    viewport.maxDepth = 1.0;

    vk::Rect2D scissor {{0, 0}, swapchainExtent};

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
    colorBlending.colorWriteMask = vk::ColorComponentFlagBits::eR
        | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
        | vk::ColorComponentFlagBits::eA;
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
        device.createPipelineLayout(vk::PipelineLayoutCreateInfo {});

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

vk::raii::Device Application::createDevice(
    vk::raii::PhysicalDevice const& physicalDevice,
    QueueFamilyIndexMap const& queueFamilyIndices,
    std::span<char const* const> deviceExtensions
) {
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

std::vector<uint32_t>
Application::readFile(std::filesystem::path const& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }

    size_t size = file.tellg();
    if (size % sizeof(uint32_t)
        != 0) { // Will make this more robust as necessary.
        throw std::runtime_error("File size is not a multiple of uint32_t!");
    }

    file.seekg(0);

    std::vector<uint32_t> fileContents;
    fileContents.resize(size / 4);
    file.read(reinterpret_cast<char*>(fileContents.data()), size);
    if (!file) {
        throw std::runtime_error("Error reading file!");
    }

    return fileContents;
}

vk::raii::ShaderModule Application::createShaderModule(
    vk::raii::Device const& device,
    std::vector<uint32_t> const& code
) {
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCode(code);
    return device.createShaderModule(createInfo);
}

std::tuple<vk::raii::PhysicalDevice, QueueFamilyIndexMap>
Application::pickPhysicalDevice(
    vk::raii::Instance const& instance,
    vk::SurfaceKHR const& surface,
    std::span<char const* const> deviceExtensions
) {
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
        if (!graphicsFamily) {
            continue;
        }

        std::optional<uint32_t> presentFamily;
        for (const auto [i, familyProperties] :
             std::views::enumerate(vFamilyProperties)) {
            if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
                presentFamily = i;
                break;
            }
        }
        if (!presentFamily) {
            continue;
        }

        auto vExtensionProperties =
            physicalDevice.enumerateDeviceExtensionProperties();

        std::unordered_set<std::string> extensionChecklist {
            deviceExtensions.cbegin(),
            deviceExtensions.cend()
        };
        for (auto const& extensionProperties : vExtensionProperties) {
            extensionChecklist.erase(extensionProperties.extensionName);
        }
        if (!extensionChecklist.empty()) {
            continue;
        }

        return std::make_tuple(
            std::move(physicalDevice),
            QueueFamilyIndexMap {
                .graphicsFamily = graphicsFamily.value(),
                .presentFamily = presentFamily.value()
            }
        );
    }

    throw std::runtime_error("No suitable GPU.");
}
