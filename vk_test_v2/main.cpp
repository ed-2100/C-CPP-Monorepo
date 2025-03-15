#include <cstring>
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>

#include "vk_engine.hpp"

int main(int argc, char *argv[]) {
    auto engine = VulkanEngine();

    engine.run();

    return 0;
}
