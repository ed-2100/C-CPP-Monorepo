// **INCOMPLETE PROJECT**

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

constexpr auto PROGRAM_NAME = "vk_test_v2";

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

// #define ENABLE_VALIDATION_LAYERS

#ifdef ENABLE_VALIDATION_LAYERS
constexpr std::array validationLayers{"VK_LAYER_KHRONOS_validation"};
#endif

class App {
  SDL_Window* window;

public:
  void run() {
    initSDL();
    main();
    cleanupSDL();
  }

private:
  void initSDL() {
    if (0 > SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
      throw std::runtime_error(
          std::format("Error initializing SDL: {}", SDL_GetError()));
    }

    window = SDL_CreateWindow("Example",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              800,
                              600,
                              SDL_WINDOW_VULKAN);
    if (nullptr == window) {
      throw std::runtime_error(
          std::format("Error creating window: {}", SDL_GetError()));
    }
  }

  void cleanupSDL() {
    // Destroy the window. This will also destroy the surface.
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();
  }

#ifdef ENABLE_VALIDATION_LAYERS
  static void checkValidationLayerSupport() {
    auto availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        throw std::runtime_error(std::format(
            "Could not find requested validation layer: {}", layerName));
      }
    }
  }
#endif

  std::vector<const char*> getRequiredExtensionNames() {
    unsigned int sdlExtensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);

    std::vector<const char*> requiredExtensionNames;

    requiredExtensionNames.reserve(sdlExtensionCount);
    SDL_Vulkan_GetInstanceExtensions(
        window, &sdlExtensionCount, requiredExtensionNames.data());

    requiredExtensionNames.push_back(
        vk::KHRPortabilityEnumerationExtensionName);

#ifdef ENABLE_VALIDATION_LAYERS
    requiredExtensionNames.push_back(vk::EXTDebugUtilsExtensionName);
#endif

    return requiredExtensionNames;
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
                VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* /*pUserData*/) {
    std::cerr << "Validation layer message: " << pCallbackData->pMessage
              << '\n';

    return VK_FALSE;
  }

  void main() {
    vk::raii::Context context;
    vk::ApplicationInfo applicationInfo;
    applicationInfo.setPApplicationName(PROGRAM_NAME);
    applicationInfo.setApplicationVersion(VK_MAKE_VERSION(0, 0, 0));
    applicationInfo.setPEngineName("None");
    applicationInfo.setEngineVersion(VK_MAKE_VERSION(0, 0, 0));
    applicationInfo.setApiVersion(VK_API_VERSION_1_3);

    auto requiredExtensions = getRequiredExtensionNames();

#ifdef ENABLE_VALIDATION_LAYERS
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    debugCreateInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning);
    debugCreateInfo.setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
    debugCreateInfo.setPfnUserCallback(debugCallback);
#endif

    vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.setFlags(
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
    instanceCreateInfo.setPApplicationInfo(&applicationInfo);
    instanceCreateInfo.setEnabledExtensionCount(
        static_cast<uint32_t>(requiredExtensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());
#ifdef ENABLE_VALIDATION_LAYERS
    instanceCreateInfo.setEnabledLayerCount(validationLayers.size());
    instanceCreateInfo.setPpEnabledLayerNames(validationLayers.data());
    instanceCreateInfo.setPNext(&debugCreateInfo);
#endif

    vk::raii::Instance instance{context, instanceCreateInfo};

#ifdef ENABLE_VALIDATION_LAYERS
    [[maybe_unused]] vk::DebugUtilsMessengerEXT debugMessenger =
        instance.createDebugUtilsMessengerEXT(debugCreateInfo);
#endif

    vk::raii::PhysicalDevices physicalDevices{instance};

    std::cout << "Number of GPUs: " << physicalDevices.size() << '\n';
    vk::raii::PhysicalDevice device = pickPhysicalDevice(physicalDevices);
    std::cout << "Using GPU: " << device.getProperties2().properties.deviceName
              << '\n';

    unsigned int extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    // glm::mat4 matrix;
    // glm::vec4 vec;
    // auto test = matrix * vec;

    bool running = true;
    while (running) {
      // Main loop here.
      std::this_thread::sleep_for(std::chrono::milliseconds(20));

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (SDL_EventType::SDL_KEYDOWN == event.type) {
          if (SDL_Scancode::SDL_SCANCODE_ESCAPE == event.key.keysym.scancode) {
            running = false;
          }
        } else if (SDL_EventType::SDL_QUIT == event.type) {
          running = false;
        }
      }
    }
  }

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() { return graphicsFamily.has_value(); }
  };

  QueueFamilyIndices findQueueFamilies(vk::raii::PhysicalDevice& device) {
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
        indices.graphicsFamily = i;
        break;
      }

      i++;
    }

    // Assign index to queue families that could be found
    return indices;
  }

  vk::raii::PhysicalDevice& pickPhysicalDevice(
      vk::raii::PhysicalDevices& physicalDevices) {
    auto candidates =
        physicalDevices | std::views::filter([this](auto&& a) {
          QueueFamilyIndices indices = findQueueFamilies(a);
          return indices.isComplete();
        }) |
        std::views::transform([](auto&& a) {
          return std::tuple<decltype(a), vk::PhysicalDeviceProperties>(
              a, a.getProperties());
        });

    auto decision = std::ranges::max_element(
        candidates, [](auto&& /*b*/, auto&& /*a*/) { return false; });

    if (candidates.end() == decision) {
      throw std::runtime_error("No suitable GPU found!");
    }

    return std::get<0>(*decision);
  }
};

int main(int /*argc*/, char** /*argv*/) {
#ifdef NDEBUG
  std::cout << "Release build.\n" << std::flush;
#else
  std::cout << "Debug build.\n" << std::flush;
#endif

  bool finally = true;
  try {
    App app{};
    app.run();
    finally = false;
  } catch (const vk::SystemError& e) {
    std::cout << "vk::SystemError: " << e.what();
  } catch (const std::exception& e) {
    std::cout << "std::exception: " << e.what();
  } catch (...) {
    std::cout << "Unknown error!";
  }
  if (finally) {
    std::cout
        << "\nExiting, because idk what else to do about this issue ATM...\n"
        << std::flush;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
