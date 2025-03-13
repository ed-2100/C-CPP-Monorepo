// **INCOMPLETE PROJECT**

// I am still in the process of modularizing this code.
// The code in `{SDLUtils,Window,Surface,SwapchainManager}.{hpp,cpp}` is nicer.

#include "Program.hpp"

#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_vulkan.h>

#include <filesystem>
#include <iostream>

int main(int /*argc*/, char const* const* argv) {
    auto exeDir = std::filesystem::canonical(std::filesystem::path(argv[0]))
                      .parent_path();
    try {
        Application renderer(exeDir);
        renderer.run();
    } catch (const std::exception& e) {
        std::cout << typeid(e).name() << ": " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown exception.";
    }
}
