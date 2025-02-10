#include "SDLUtils.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

#include <stdexcept>

SDLContext::SDLContext() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw std::runtime_error(std::string("Failed to init SDL: ") +
                             SDL_GetError());
  }
}
SDLContext::~SDLContext() { SDL_Quit(); }

SDLWindow::SDLWindow(SDLContext& context,
                     char const* name,
                     uint32_t w,
                     uint32_t h) {
  handle = SDL_CreateWindow(name, w, h, SDL_WINDOW_VULKAN);
}
SDLWindow::~SDLWindow() {
  if (handle) {
    SDL_DestroyWindow(handle);
  }
}

SDLSurface::SDLSurface(SDLWindow const& window,
                       vk::raii::Instance const& instance)
    : Surface(createSurface(window, instance)), instance(instance) {}
SDLSurface::~SDLSurface() {
  if (surface) {
    SDL_Vulkan_DestroySurface(vk::Instance(instance), surface, nullptr);
  }
}
VkSurfaceKHR SDLSurface::createSurface(SDLWindow const& window,
                                       vk::Instance instance) {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
    throw std::runtime_error("Failed to create window!");
  }
  return surface;
}
