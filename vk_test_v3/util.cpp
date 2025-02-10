#include "util.hpp"
#include <SDL3/SDL_vulkan.h>

vk::raii::SurfaceKHR SDLWindow::createSurface(vk::raii::Instance const& instance) {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(
          handle, static_cast<VkInstance>(*instance), nullptr, &surface)) {
    throw std::runtime_error("Failed to create window!");
  }
  std::cout << "@S" << std::endl;
  return vk::raii::SurfaceKHR(instance, surface);
}

SDLWindow SDLContext::createWindow(std::string const& windowName,
                                 vk::Extent2D const& extent) const {
  SDL_Window* window = SDL_CreateWindow(
      windowName.c_str(), extent.width, extent.height, SDL_WINDOW_VULKAN);

  std::cout << "@W" << std::endl;
  return SDLWindow(*this, window, windowName, extent);
}
