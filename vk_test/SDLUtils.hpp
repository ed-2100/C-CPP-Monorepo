#pragma once

#include "Surface.hpp"
#include "Window.hpp"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan_raii.hpp>

struct SDLContext;
struct SDLWindow;
struct SDLSurface;

struct SDLContext {
    SDLContext();
    ~SDLContext();

    SDLContext(SDLContext const&) = delete;
    SDLContext& operator=(SDLContext const&) = delete;

    SDLContext(SDLContext&&) = default;
    SDLContext& operator=(SDLContext&&) = default;

    static std::shared_ptr<SDLContext> getInstance();

    std::span<char const* const> getInstanceExtensions() const;
};

struct SDLWindow final : public Window {
    SDLWindow() = delete;
    constexpr SDLWindow(std::nullptr_t) noexcept {}
    SDLWindow(char const* name, uint32_t w, uint32_t h);
    ~SDLWindow();

    SDLWindow(SDLWindow&) = delete;
    SDLWindow& operator=(SDLWindow&) = delete;

    SDLWindow(SDLWindow&& rhs) noexcept :
        Window(std::move(rhs)), handle(std::exchange(rhs.handle, {})) {}
    SDLWindow& operator=(SDLWindow&& rhs) noexcept {
        if (this != &rhs) {
            Window::operator=(std::move(rhs));

            // This is necessary, as the ptr would be copied otherwise.
            handle = std::exchange(rhs.handle, {});
        }
        return *this;
    };

    VkExtent2D queryExtent() const {
        int width, height;
        SDL_GetWindowSize(handle, &width, &height);

        return VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    constexpr operator SDL_Window*() const { return handle; }

    SDL_Window* handle = {};
};

struct SDLSurface final : public Surface {
    SDLSurface() = delete;
    SDLSurface(std::nullptr_t) : Surface(nullptr) {}
    SDLSurface(SDLWindow const& window, vk::Instance instance);
    ~SDLSurface() override;

    SDLSurface(SDLSurface&) = delete;
    SDLSurface& operator=(SDLSurface&) = delete;

    SDLSurface(SDLSurface&& rhs) noexcept :
        Surface(std::move(rhs)), instance(std::exchange(rhs.instance, {})) {}
    SDLSurface& operator=(SDLSurface&& rhs) noexcept {
        if (this != &rhs) {
            Surface::operator=(std::move(rhs));
            instance = std::exchange(rhs.instance, {});
        }
        return *this;
    }

    vk::SurfaceKHR createSurface(SDLWindow const& window, vk::Instance instance);

    vk::Instance instance = {};
};
