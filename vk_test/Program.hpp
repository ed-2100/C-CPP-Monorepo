#pragma once

#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include "SDLUtils.hpp"

union QueueFamilyIndexMap {
  struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
  };

  uint32_t families[2];
};

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
