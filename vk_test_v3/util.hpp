#pragma once

#include <iostream>

#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

struct Window;

struct glfwContext {
  glfwContext() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      throw std::runtime_error(std::string("Failed to init SDL: ") +
                               SDL_GetError());
    }
    std::cout << "@G" << std::endl;
  }
  ~glfwContext() {
    SDL_Quit();
    std::cout << "DG" << std::endl;
  }

  glfwContext(glfwContext const& rhs) = delete;
  glfwContext& operator=(glfwContext const& rhs) = delete;

  glfwContext(glfwContext&& rhs) = delete;
  glfwContext& operator=(glfwContext&& other) = delete;

  Window createWindow(std::string const& windowName,
                      vk::Extent2D const& extent) const;
};

struct Window {
  Window() = delete;
  ~Window() noexcept {
    if (handle) {
      SDL_DestroyWindow(handle);
      std::cout << "DW" << std::endl;
    }
  }

  Window(glfwContext const& context,
         SDL_Window* window,
         std::string const& name,
         vk::Extent2D const& extent)
      : context(context), handle(window), name(name), extent(extent) {}

  Window(Window const&) = delete;
  Window& operator=(Window const&) = delete;

  Window(Window&& rhs) = delete;
  Window& operator=(Window&& rhs) = delete;

  vk::raii::SurfaceKHR createSurface(vk::raii::Instance const& instance);

  constexpr SDL_Window* getHandle() const noexcept { return handle; }

private:
  [[maybe_unused]]
  const glfwContext& context;
  SDL_Window* handle = nullptr;
  std::string name;
  vk::Extent2D extent;
};

// Probably the most baller `todo!()` implementation there ever was.
struct TODO {
  [[noreturn]] TODO() { throw std::logic_error("TODO: Not implemented!"); }

  template <typename T>
  [[noreturn]] operator T() const {
    throw std::logic_error("TODO: This should be unreachable.");
  }
};

template <typename T>
inline T expect(std::optional<T> wrapped, char const* msg) {
  if (!wrapped) {
    throw std::runtime_error(msg);
  }

  return wrapped.value();
}
