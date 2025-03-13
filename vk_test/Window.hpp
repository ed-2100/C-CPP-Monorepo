#pragma once

#include <vulkan/vulkan_core.h>

struct Window {
  constexpr Window() {}
  virtual ~Window() = 0;

  Window(Window const&) = delete;
  Window& operator=(Window const&) = delete;

  Window(Window&&) = default;
  Window& operator=(Window&&) = default;

  virtual VkExtent2D queryExtent() const = 0;
};

constexpr Window::~Window() {}
