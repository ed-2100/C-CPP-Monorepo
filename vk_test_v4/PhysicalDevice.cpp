#include "PhysicalDevice.hpp"

#include <memory>
#include <ranges>
#include <unordered_set>

std::tuple<VkPhysicalDevice, QueueFamilyIndexMap> pickPhysicalDevice(
    VkInstance instance,
    VkSurfaceKHR surface,
    std::span<char const* const> const& deviceExtensions) {
  uint32_t physicalDeviceCount;
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

  auto physicalDevices =
      std::make_unique<VkPhysicalDevice[]>(physicalDeviceCount);
  auto physicalDevices_span =
      std::span(physicalDevices.get(), physicalDeviceCount);

  vkEnumeratePhysicalDevices(
      instance, &physicalDeviceCount, physicalDevices.get());

  for (VkPhysicalDevice physicalDevice : physicalDevices_span) {
    uint32_t queueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr);

    auto queueFamilyProperties =
        std::make_unique<VkQueueFamilyProperties[]>(queueFamilyPropertyCount);
    auto queueFamilyProperties_span =
        std::span(queueFamilyProperties.get(), queueFamilyPropertyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.get());

    std::optional<uint32_t> graphicsFamily;
    for (const auto [i, queueFamilyProperties] :
         std::views::enumerate(queueFamilyProperties_span)) {
      if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        graphicsFamily = i;
        break;
      }
    }
    if (!graphicsFamily) continue;

    std::optional<uint32_t> presentFamily;
    for (const auto [i, queueFamilyProperties] :
         std::views::enumerate(queueFamilyProperties_span)) {
      VkBool32 supported;
      VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
          physicalDevice, i, surface, &supported);
      if (result != VK_SUCCESS) {
        throw std::runtime_error(
            std::string(
                "Failed to call vkGetPhysicalDeviceSurfaceSupportKHR: ") +
            std::to_string(result));
      }
      if (supported) {
        presentFamily = i;
        break;
      }
    }
    if (!presentFamily) continue;

    uint32_t propertyCount;
    vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &propertyCount, nullptr);

    auto properties = std::make_unique<VkExtensionProperties[]>(propertyCount);
    auto properties_span = std::span(properties.get(), propertyCount);

    vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &propertyCount, properties.get());

    std::unordered_set<std::string_view> extensionChecklist{
        deviceExtensions.begin(), deviceExtensions.end()};
    for (auto extensionProperties : properties_span) {
      extensionChecklist.erase(extensionProperties.extensionName);
    }
    if (!extensionChecklist.empty()) continue;

    QueueFamilyIndexMap queueFamilyIndexMap{
        .graphicsFamily = graphicsFamily.value(),
        .presentFamily = presentFamily.value(),
    };

    return std::make_tuple(physicalDevice, queueFamilyIndexMap);
  }

  throw std::runtime_error(
      "Failed to find a GPU with the required capabilities!");
}
