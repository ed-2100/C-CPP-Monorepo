#include "vk_engine.hpp"
#include "vk_images.hpp"
#include "vk_initializers.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include <magic_enum/magic_enum.hpp>

#ifdef NDEBUG // Release mode
constexpr bool enableValidationLayers = false;
#else // Debug mode
constexpr bool enableValidationLayers = true;
#endif

static bool engineLoaded = false;

void checkVkResult(VkResult result) {
    if (result) {
        throw std::runtime_error(
            std::format("Encountered Vulkan error code: {}", magic_enum::enum_name(result))
        );
    }
}

VulkanEngine::VulkanEngine() {
    // Happens if we try to create more than one engine
    assert(!engineLoaded);
    engineLoaded = true;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Vulkan Engine",
        window_extent.width,
        window_extent.height,
        SDL_WINDOW_VULKAN
    );
    assert(window);

    init_vulkan();

    init_swapchain();

    init_commands();

    init_sync_structures();
}

VulkanEngine::~VulkanEngine() {
    if (device) {
        vkDeviceWaitIdle(device);
    }

    for (auto &frame : frames) {
        if (frame.command_pool) {
            vkDestroyCommandPool(device, frame.command_pool, nullptr);
        }
    }

    destroy_swapchain();

    if (device) {
        vkDestroyDevice(device, nullptr);
    }

    if (surface) {
        assert(instance);
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    if (debug_messenger) {
        assert(instance);
        vkb::destroy_debug_utils_messenger(instance, debug_messenger, nullptr);
    }

    if (instance) {
        vkDestroyInstance(instance, nullptr);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    engineLoaded = false;
}

void VulkanEngine::run() {
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastPrintTime = startTime;
    auto loopTime = startTime;

    bool quit = false;
    while (!quit) {
        std::cout << "." << std::endl;
        draw();

        std::this_thread::sleep_until(loopTime + std::chrono::milliseconds(16));

        auto endTime = std::chrono::high_resolution_clock::now();

        if (endTime - lastPrintTime >= std::chrono::milliseconds(100)) {
            using seconds_f32 = std::chrono::duration<float, std::chrono::seconds::period>;
            auto deltaTime = seconds_f32(endTime - loopTime).count();
            auto fps = 1.0f / deltaTime;
            std::cout << std::format("FPS: {:8.2f}\n", fps);
            lastPrintTime = endTime;
        }

        loopTime = endTime;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
    }

    vkDeviceWaitIdle(device);
}

void VulkanEngine::draw() {
    checkVkResult(
        vkWaitForFences(device, 1, &get_current_frame().render_fence, true, 1000000000000000000)
    );
    checkVkResult(vkResetFences(device, 1, &get_current_frame().render_fence));

    uint32_t swapchain_image_index;
    checkVkResult(vkAcquireNextImageKHR(
        device,
        swapchain,
        1000000000000000000,
        get_current_frame().swaphcain_semaphore,
        nullptr,
        &swapchain_image_index
    ));

    VkCommandBuffer cmd = get_current_frame().main_command_buffer;

    checkVkResult(vkResetCommandBuffer(cmd, 0));

    auto cmd_begin_info = VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    checkVkResult(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    vkutil::transition_image(
        cmd,
        swapchain_images[swapchain_image_index],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL
    );

    float flash = std::abs(std::sin(frame_number / 120.f));
    auto clear_value = VkClearColorValue{{0.0f, 0.0f, flash, 1.0f}};

    VkImageSubresourceRange clear_range =
        vkutil::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    vkCmdClearColorImage(
        cmd,
        swapchain_images[swapchain_image_index],
        VK_IMAGE_LAYOUT_GENERAL,
        &clear_value,
        1,
        &clear_range
    );

    vkutil::transition_image(
        cmd,
        swapchain_images[swapchain_image_index],
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    checkVkResult(vkEndCommandBuffer(cmd));
}

void VulkanEngine::init_vulkan() {
    // ----- Instance Creation -----
    vkb::InstanceBuilder builder;

    // clang-format off
    auto inst_ret = builder.set_app_name("Vulkan Engine")
        .request_validation_layers(enableValidationLayers)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();
    // clang-format on

    vkb::Instance vkb_inst = inst_ret.value();

    instance = vkb_inst.instance;
    debug_messenger = vkb_inst.debug_messenger;

    // ----- Surface Creation -----
    SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);

    // ----- Features -----
    auto vk12features = VkPhysicalDeviceVulkan12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .bufferDeviceAddress = true,
    };

    auto vk13features = VkPhysicalDeviceVulkan13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .synchronization2 = true,
        .dynamicRendering = true,
    };

    // ----- Physical Device Selection -----
    vkb::PhysicalDeviceSelector selector{vkb_inst};

    // clang-format off
    vkb::PhysicalDevice vkb_physical_device = selector.set_surface(surface)
        .set_minimum_version(1, 3)
        .set_required_features_12(vk12features)
        .set_required_features_13(vk13features)
        .select()
        .value();
    // clang-format on

    physical_device = vkb_physical_device.physical_device;

    // ----- Device Creation -----
    vkb::DeviceBuilder device_builder{vkb_physical_device};
    vkb::Device device_ret = device_builder.build().value();

    device = device_ret.device;

    // ----- Queue Creation -----
    graphics_queue = device_ret.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_family = device_ret.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::init_swapchain() {
    create_swapchain();
}

void VulkanEngine::create_swapchain() {
    vkb::SwapchainBuilder swapchain_builder{physical_device, device, surface};
    vkb::Swapchain vkb_swapchain =
        swapchain_builder
            .set_desired_format(VkSurfaceFormatKHR{
                .format = swapchain_image_format,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            })
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(window_extent.width, window_extent.height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();

    swapchain_extent = vkb_swapchain.extent;
    swapchain = vkb_swapchain.swapchain;
    swapchain_images = vkb_swapchain.get_images().value();
    swapchain_image_views = vkb_swapchain.get_image_views().value();
}

void VulkanEngine::destroy_swapchain() {
    for (auto &image_view : swapchain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }
    swapchain_image_views.clear();

    if (swapchain) {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }
}

void VulkanEngine::init_commands() {
    auto command_pool_info = VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_family,
    };

    for (size_t i = 0; i < FRAME_OVERLAP; i++) {
        checkVkResult(
            vkCreateCommandPool(device, &command_pool_info, nullptr, &frames[i].command_pool)
        );

        auto cmd_alloc_info = VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = frames[i].command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        checkVkResult(
            vkAllocateCommandBuffers(device, &cmd_alloc_info, &frames[i].main_command_buffer)
        );
    }
}

void VulkanEngine::init_sync_structures() {
    auto fence_create_info = VkFenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    auto semaphore_create_info = VkSemaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    for (auto &frame : frames) {
        checkVkResult(vkCreateFence(device, &fence_create_info, nullptr, &frame.render_fence));

        checkVkResult(
            vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frame.swaphcain_semaphore)
        );
        checkVkResult(
            vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frame.render_semaphore)
        );
    }
}

FrameData &VulkanEngine::get_current_frame() {
    return frames[frame_number % FRAME_OVERLAP];
};
