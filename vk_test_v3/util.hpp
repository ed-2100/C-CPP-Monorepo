#pragma once

#include <iostream>

#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct Window;

struct glfwContext {
  glfwContext() {
    glfwInit();
    glfwSetErrorCallback([](int error, char const* msg) {
      std::cerr << "glfw: "
                << "(" << error << ") " << msg << std::endl;
    });
    std::cout << "@G" << std::endl;
  }
  ~glfwContext() {
    glfwTerminate();
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
      glfwDestroyWindow(handle);
      std::cout << "DW" << std::endl;
    }
  }

  Window(glfwContext const& context,
         GLFWwindow* window,
         std::string const& name,
         vk::Extent2D const& extent)
      : context(context), handle(window), name(name), extent(extent) {}

  Window(Window const&) = delete;
  Window& operator=(Window const&) = delete;

  Window(Window&& rhs) = delete;
  Window& operator=(Window&& rhs) = delete;

  vk::raii::SurfaceKHR createSurface(vk::raii::Instance const& instance);

  constexpr GLFWwindow* getHandle() const noexcept { return handle; }

private:
  [[maybe_unused]]
  const glfwContext& context;
  GLFWwindow* handle = nullptr;
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
