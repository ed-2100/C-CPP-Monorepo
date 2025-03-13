#pragma once

#include <vulkan/vulkan_core.h>

struct Window {
    Window() {}
    virtual ~Window() = 0;

    Window(Window const&) = delete;
    Window& operator=(Window const&) = delete;

    Window(Window&&) = default;
    Window& operator=(Window&&) = default;

    virtual VkExtent2D queryExtent() const = 0;
};

inline Window::~Window() {}
