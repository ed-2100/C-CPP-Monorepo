#include "Debug.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "SDLUtils.hpp"
// #include "PhysicalDevice.hpp"
// #include "Device.hpp"
// #include "Swapchain.hpp"
// #include "Pipeline.hpp"

#include <SDL3/SDL_events.h>

#include <thread>

const char* pApplicationName = "Triangle";

const std::array deviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

int main() {
  // Small beginnings...
  SDLContext context;
  SDLWindow window{context, pApplicationName, 500, 500};

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo =
      createDebugUtilsMessengerCreateInfo();

  Instance instance(pApplicationName, &debugCreateInfo);

  DebugMessenger messenger(instance, debugCreateInfo);

  SDLSurface surface{window, instance};

  // VkPhysicalDevices don't need to be deconstructed,
  // but it might be prudent to encapsulate it's associated
  // variables with it for maintainability.
  auto [physicalDevice, queueFamilyIndexMap] =
      pickPhysicalDevice(instance, surface, deviceExtensions);

  Device device{physicalDevice, deviceExtensions, queueFamilyIndexMap.families};



  VkQueue graphicsQueue, presentQueue;
  vkGetDeviceQueue(device, queueFamilyIndexMap.graphicsFamily, 0, &graphicsQueue);
  vkGetDeviceQueue(device, queueFamilyIndexMap.presentFamily, 0, &presentQueue);


  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      } else if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          done = true;
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return 0;
}
