#include "Device.hpp"

#include <unordered_set>
#include <vector>

Device::Device(VkPhysicalDevice physicalDevice,
               std::span<char const* const> const& deviceExtensions,
               std::span<uint32_t const> const& queueFamilyIndices) {
  std::unordered_set<uint32_t> uniqueFamilyIndices{queueFamilyIndices.begin(),
                                                   queueFamilyIndices.end()};

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  queueCreateInfos.reserve(uniqueFamilyIndices.size());
  float priority = 1.0;
  for (auto queueFamilyIndex : uniqueFamilyIndices) {
    VkDeviceQueueCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    queueCreateInfos.push_back(createInfo);
  }

  VkPhysicalDeviceFeatures features{};

  VkDeviceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data(),
      .pEnabledFeatures = &features,
  };

  vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
}
Device::~Device() { vkDestroyDevice(device, nullptr); }
