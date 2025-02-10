#include "Debug.hpp"

#include <iostream>
#include <stdexcept>

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
              VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
              VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
              void* /*pUserData*/) {
  std::cerr << "Validation layer message: " << pCallbackData->pMessage << '\n';

  return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT createDebugUtilsMessengerCreateInfo() {
  return VkDebugUtilsMessengerCreateInfoEXT{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debugCallback,
  };
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger) {
  static PFN_vkCreateDebugUtilsMessengerEXT func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          instance, "vkCreateDebugUtilsMessengerEXT");
  if (func == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Failed to call vkGetInstanceProcAddr for "
        "vkCreateDebugUtilsMessengerEXT!");
  }

  return func(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL
vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                VkDebugUtilsMessengerEXT messenger,
                                const VkAllocationCallbacks* pAllocator) {
  static PFN_vkDestroyDebugUtilsMessengerEXT func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Failed to call vkGetInstanceProcAddr for "
        "vkDestroyDebugUtilsMessengerEXT!");
  }

  return func(instance, messenger, pAllocator);
}

DebugMessenger::DebugMessenger(
    VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const& createInfo)
    : instance(instance) {
  VkResult result = vkCreateDebugUtilsMessengerEXT(
      instance, &createInfo, nullptr, &messenger);
  if (result != VK_SUCCESS) {
    throw std::runtime_error(
        std::string("Failed to create DebugUtilsMessengerEXT: ") +
        std::to_string(result));
  }
}
DebugMessenger::~DebugMessenger() {
  if (messenger) {
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
  }
}
