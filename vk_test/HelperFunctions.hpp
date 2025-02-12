#pragma once

#include "SDLUtils.hpp"

union QueueFamilyIndexMap {
  struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
  };

  uint32_t families[2];
};

#include <SDL3/SDL_vulkan.h>
#include <filesystem>

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
              VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
              VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
              void* /*pUserData*/);
vk::raii::Instance createInstance(SDLContext const& sdlContext,
                                  vk::raii::Context const& context,
                                  char const* appName,
                                  std::span<char const* const> validationLayers,
                                  void* pNext);
vk::raii::RenderPass createRenderPass(
    vk::raii::Device const& device, vk::SurfaceFormatKHR const& surfaceFormat);
vk::raii::Pipeline createGraphicsPipeline(
    vk::raii::Device const& device,
    vk::Extent2D const& swapchainExtent,
    vk::raii::RenderPass const& renderPass,
    std::filesystem::path const& executableDirectory);
std::tuple<vk::raii::PhysicalDevice, QueueFamilyIndexMap> pickPhysicalDevice(
    vk::raii::Instance const& instance,
    vk::SurfaceKHR const& surface,
    std::span<char const* const> deviceExtensions);
vk::raii::Device createDevice(vk::raii::PhysicalDevice const& physicalDevice,
                              QueueFamilyIndexMap const& queueFamilyIndices,
                              std::span<char const* const> deviceExtensions);
std::vector<uint32_t> readFile(std::filesystem::path const& filename);
vk::raii::ShaderModule createShaderModule(vk::raii::Device const& device,
                                          std::vector<uint32_t> const& code);
