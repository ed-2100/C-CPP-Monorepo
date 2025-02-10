#pragma once

#include "Surface.hpp"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

struct SDLContext;
struct SDLWindow;
struct SDLSurface;

struct SDLContext {
  SDLContext();
  ~SDLContext();

  SDLContext(SDLContext const&) = delete;
  SDLContext& operator=(SDLContext const&) = delete;

  SDLContext(SDLContext const&&) = delete;
  SDLContext& operator=(SDLContext const&&) = delete;
};

struct SDLWindow {
  SDLWindow() = delete;
  SDLWindow(SDLContext& context, char const* name, uint32_t w, uint32_t h);
  ~SDLWindow();

  SDLWindow(SDLWindow const&) = delete;
  SDLWindow& operator=(SDLWindow const&) = delete;

  SDLWindow(SDLWindow const&&) = delete;
  SDLWindow& operator=(SDLWindow const&&) = delete;

  constexpr operator SDL_Window*() const { return handle; }

  SDL_Window* handle;
};

struct SDLSurface final : public Surface {
  SDLSurface() = delete;
  SDLSurface(SDL_Window* window, VkInstance instance);
  ~SDLSurface() override final;

  VkSurfaceKHR createSurface(SDL_Window* window, VkInstance instance);

  VkInstance instance;
};
