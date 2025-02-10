#include "PhysicalDevice.hpp"

#include <memory>
#include <ranges>
#include <unordered_set>
#include "Instance.hpp"

VkSurfaceCapabilitiesKHR PhysicalDevice::getSurfaceCapabilities(
    VkSurfaceKHR surface) const {
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &surfaceCapabilities);
  return surfaceCapabilities;
}

std::vector<VkSurfaceFormatKHR> PhysicalDevice::getSurfaceFormats(
    VkSurfaceKHR surface) const {
  uint32_t surfaceFormatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      physicalDevice, surface, &surfaceFormatCount, nullptr);

  auto surfaceFormats = std::vector<VkSurfaceFormatKHR>(surfaceFormatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

  return surfaceFormats;
}

std::vector<VkPresentModeKHR> PhysicalDevice::getSurfacePresentModes(
    VkSurfaceKHR surface) const {
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &presentModeCount, nullptr);

  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &presentModeCount, presentModes.data());

  return presentModes;
}

std::tuple<PhysicalDevice, QueueFamilyIndexMap> pickPhysicalDevice(
    Instance const& instance,
    VkSurfaceKHR surface,
    std::span<char const* const> const& deviceExtensions) {

  for (VkPhysicalDevice physicalDevice : instance.enumeratePhysicalDevices()) {
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
