#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdlib>
#include <iostream>

class App {
    GLFWwindow *window;

public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow();

    void initVulkan() {}

    void mainLoop() {
        auto startTime = std::chrono::high_resolution_clock::now();

        auto lastPrintTime = startTime;

        auto lastLoopTime = startTime;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // Stuff here

            auto loopTime = std::chrono::high_resolution_clock::now();

            auto deltaTime =
                std::chrono::duration_cast<std::chrono::duration<float>>(lastLoopTime - loopTime);

            if (std::chrono::duration_cast<std::chrono::duration<float>>(lastLoopTime -
                                                                         lastPrintTime)
                    .count() >= 1.0) {
                std::cout << "FPS: " << 1.0 / deltaTime.count() << std::endl;
                lastPrintTime = lastLoopTime;
            }

            lastLoopTime = loopTime;
        }
    }

    void cleanup() {
        glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main() {
    App app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void App::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(500, 500, "Window Title Placeholder", nullptr, nullptr);
}
