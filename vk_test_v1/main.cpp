// **INCOMPLETE PROJECT**

#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr std::array validationLayers{"VK_LAYER_KHRONOS_validation"};

class App {
  SDL_Window* window;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice =
      VK_NULL_HANDLE; // Destroyed with `instance`.

public:
  void run() {
    initSDL();
    initVulkan();
    mainLoop();
    cleanupVulkan();
    cleanupSDL();
  }

private:
  void initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
      throw std::runtime_error(
          std::format("Error initializing SDL: {}", SDL_GetError()));
    }

    window = SDL_CreateWindow("Example", 800, 600, SDL_WINDOW_VULKAN);
    if (nullptr == window) {
      throw std::runtime_error(
          std::format("Error creating window: {}", SDL_GetError()));
    }
  }

  void cleanupSDL() {
    // Destroy the window. This will also destroy the surface
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();
  }

  static void checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (auto layerName : validationLayers) {
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

  std::vector<const char*> getRequiredExtensionNames() {
    unsigned int sdlExtensionCount;
    char const* const* sdlExtensions =
        SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

    std::vector<const char*> requiredExtensionNames;

    requiredExtensionNames.reserve(sdlExtensionCount);
    for (auto sdlExtension : std::span(sdlExtensions, sdlExtensionCount)) {
      requiredExtensionNames.push_back(sdlExtension);
    }

    requiredExtensionNames.push_back(
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    if constexpr (enableValidationLayers) {
      requiredExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return requiredExtensionNames;
  }

  void createInstance() {
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    auto requiredExtensionNames = getRequiredExtensionNames();

    // TODO: Check for optional extension support and request it as well.

    std::cout << "Requested extensions:\n";
    for (const char* name : requiredExtensionNames) {
      std::cout << "    " << name << '\n';
    }
    std::cout << std::flush;

    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount =
            static_cast<uint32_t>(requiredExtensionNames.size()),
        .ppEnabledExtensionNames = requiredExtensionNames.data(),
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if constexpr (enableValidationLayers) {
      checkValidationLayerSupport(); // Will throw if one is missing.
      createInfo.enabledLayerCount = validationLayers.size();
      createInfo.ppEnabledLayerNames = validationLayers.data();
      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create VkInstance!");
    }
  }

  static VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
      const VkAllocationCallbacks* pAllocator,
      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    static PFN_vkCreateDebugUtilsMessengerEXT func = nullptr;

    if (nullptr == func) {
      func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    }

    if (nullptr == func) {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }

  static void DestroyDebugUtilsMessengerEXT(
      VkInstance instance,
      VkDebugUtilsMessengerEXT debugMessenger,
      const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDebugUtilsMessengerEXT func = nullptr;

    if (nullptr == func) {
      func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    }

    if (nullptr != func) {
      func(instance, debugMessenger, pAllocator);
    }
  }

  static void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };
  }

  void setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(
            instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }

  void initVulkan() {
    createInstance();
    if constexpr (enableValidationLayers) {
      setupDebugMessenger();
    }
    pickPhysicalDevice();
  }

  static bool isDeviceSuitable(VkPhysicalDevice /*device*/) { return true; }

  void pickPhysicalDevice() {
    unsigned int deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (0 == deviceCount) {
      throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
    }

    auto devices = std::make_unique<VkPhysicalDevice[]>(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.get());

    for (unsigned int i = 0; i < deviceCount; i++) {
      if (isDeviceSuitable(devices[i])) {
        physicalDevice = devices[i];
        break;
      }
    }
    if (VK_NULL_HANDLE == physicalDevice) {
      throw std::runtime_error("Failed to find a suitable GPU!");
    }
  }

  void cleanupVulkan() {
    if constexpr (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
                VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* /*pUserData*/) {
    std::cerr << "Validation layer message: " << pCallbackData->pMessage << '\n'
              << std::flush;

    return VK_FALSE;
  }

  static void mainLoop() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    // glm::mat4 matrix;
    // glm::vec4 vec;
    // auto test = matrix * vec;

    bool running = true;
    while (running) {
      // Main loop here.

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
          if (event.key.key == SDLK_ESCAPE) {
            running = false;
          }
        } else if (event.type == SDL_EVENT_QUIT) {
          running = false;
        }
      }
    }
  }
};

int main(int /*argc*/, char** /*argv*/) {
#ifdef NDEBUG
  std::cout << "Release build.\n" << std::flush;
#else
  std::cout << "Debug build.\n" << std::flush;
#endif

  try {
    App app;
    app.run();
  } catch (const std::exception& e) {
    std::cout << "Caught exception: " << e.what() << "\nExiting...\n"
              << std::flush;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
