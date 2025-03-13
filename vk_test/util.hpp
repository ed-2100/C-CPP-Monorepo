#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

// Probably the most baller `todo!()` implementation there ever was.
struct TODO {
    [[noreturn]] TODO() {
        throw std::logic_error("TODO: Not implemented!");
    }

    template <typename T> [[noreturn]] operator T() const {
        throw std::logic_error("TODO: This should be unreachable.");
    }
};

template <typename T> inline T expect(std::optional<T> wrapped, char const *msg) {
    if (!wrapped) {
        throw std::runtime_error(msg);
    }

    return wrapped.value();
}
