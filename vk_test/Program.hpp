#pragma once

#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include "SDLUtils.hpp"

#include "SwapchainManager.hpp"
#include "Types.hpp"

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
  Renderer(std::filesystem::path const& exeDir);

  void run();

private:
  static vk::raii::Instance createInstance(SDLContext const& sdlContext,
                                           vk::raii::Context& context,
                                           void* pNext);
  static vk::raii::RenderPass createRenderPass(
      vk::raii::Device const& device,
      vk::SurfaceFormatKHR const& surfaceFormat);
  static vk::raii::Pipeline createGraphicsPipeline(
      vk::raii::Device const& device,
      vk::Extent2D const& swapchainExtent,
      vk::raii::RenderPass const& renderPass,
      std::filesystem::path const& exeDir);
  static vk::raii::Device createDevice(
      vk::raii::PhysicalDevice const& physicalDevice,
      QueueFamilyIndexMap const& queueFamilyIndices,
      std::span<char const* const> deviceExtensions);
  static std::vector<uint32_t> readFile(std::filesystem::path const& filename);
  static vk::raii::ShaderModule createShaderModule(
      vk::raii::Device const& device, std::vector<uint32_t> const& code);
  static std::tuple<vk::raii::PhysicalDevice, QueueFamilyIndexMap>
  pickPhysicalDevice(vk::raii::Instance const& instance,
                     vk::SurfaceKHR const& surface,
                     std::span<char const* const> deviceExtensions);
};
