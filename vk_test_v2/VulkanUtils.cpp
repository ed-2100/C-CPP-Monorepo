#include "VulkanUtils.hpp"

#include <vulkan/vulkan_core.h>
#include <cstddef>
#include <cstring>
#include <ranges>

namespace vke {

// ----- Instance -----

Instance::Inner::Inner(const VkInstanceCreateInfo& create_info)
    : instance([&create_info]() {
          VkInstance instance;
          checkResult(vkCreateInstance(&create_info, nullptr, &instance));

          return instance;
      }()),
      vkCreateDebugUtilsMessengerEXT((PFN_vkCreateDebugUtilsMessengerEXT
      )vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")),
      vkDestroyDebugUtilsMessengerEXT((PFN_vkDestroyDebugUtilsMessengerEXT
      )vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")) {}

Instance::Inner::~Inner() {
    vkDestroyInstance(instance, nullptr);
}

Instance InstanceBuilder::build() {
    auto app_info = VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
        .pEngineName = "Vulkan Engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    auto create_info = VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = pNext,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    return Instance(create_info);
}

InstanceBuilder& InstanceBuilder::with_validation_layers() {
    this->validation_layers = true;
    layers.push_back("VK_LAYER_KHRONOS_validation");
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return *this;
}

InstanceBuilder& InstanceBuilder::with_extensions(std::span<const char* const> extensions) {
    for (const auto& extension : extensions) {
        this->extensions.push_back(extension);
    }
    return *this;
}

InstanceBuilder& InstanceBuilder::with_layers(std::span<const char* const> layers) {
    for (const auto& layer : layers) {
        this->layers.push_back(layer);
    }
    return *this;
}

// ----- DebugUtilsMessengerEXT -----

DebugUtilsMessengerEXT::Inner::Inner(
    Instance instance,
    const VkDebugUtilsMessengerCreateInfoEXT& create_info
)
    : instance(instance), debug_messenger([&instance, &create_info]() {
          if (!instance->vkCreateDebugUtilsMessengerEXT ||
              !instance->vkDestroyDebugUtilsMessengerEXT) {
              checkResult(VK_ERROR_EXTENSION_NOT_PRESENT);
          }

          VkDebugUtilsMessengerEXT debug_messenger;
          checkResult(instance->vkCreateDebugUtilsMessengerEXT(
              instance,
              &create_info,
              nullptr,
              &debug_messenger
          ));

          return debug_messenger;
      }()) {}

DebugUtilsMessengerEXT::Inner::~Inner() {
    instance->vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
}

// ----- PhysicalDeviceSelector -----

PhysicalDeviceSelector::Node::Node(size_t size) : size(size), next(nullptr) {
    user = new std::byte[size];
    api = new std::byte[size];
}

PhysicalDeviceSelector::Node::~Node() {
    delete[] user;
    delete[] api;
}

PhysicalDeviceSelector::~PhysicalDeviceSelector() {
    while (head) {
        Node* next = head->next;
        delete head;
        head = next;
    }
}

void PhysicalDeviceSelector::with_features_struct_inner(std::byte* to_add, size_t size) {
    Node* next = head;

    while (next) {
        if (next->user_sType() == *(VkStructureType*)to_add) {
            memcpy(next->user + offset_first, &to_add, size - offset_first);
            return;
        }

        next = next->next;
    }

    Node* newNode = new Node(size);
    memcpy(newNode->user, to_add, size);
    newNode->user_sType() = *(VkStructureType*)to_add;
    newNode->api_sType() = *(VkStructureType*)to_add;
    if (!head) {
        newNode->user_pNext() = nullptr;
        newNode->api_pNext() = nullptr;
        newNode->next = nullptr;
    } else {
        newNode->user_pNext() = head->user;
        newNode->api_pNext() = head->api;
        newNode->next = head;
    }
    head = newNode;
}

bool PhysicalDeviceSelector::is_suitable(VkPhysicalDevice physical_device) const {
    Node node = Node(sizeof(features));
    memcpy(node.user, &features, node.size);
    node.user_sType() = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    node.api_sType() = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    node.user_pNext() = head->user;
    node.api_pNext() = head->api;
    node.next = head;

    vkGetPhysicalDeviceFeatures2(physical_device, (VkPhysicalDeviceFeatures2*)node.api);

    Node* next = &node;

    while (next) {
        for (auto [user_flag, api_flag] : next->get_bool_pairs()) {
            if (user_flag) {
                if (!api_flag) { return false; }
            }
        }

        next = next->next;
    }

    return true;
}

std::vector<VkPhysicalDevice> PhysicalDeviceSelector::select(VkInstance instance) const {
    uint32_t physical_device_count;
    checkResult(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    checkResult(
        vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data())
    );

    std::vector<VkPhysicalDevice> suitable_physical_devices =
        physical_devices | std::ranges::views::filter([this](auto dev) {
            return is_suitable(dev);
        }) |
        std::ranges::to<std::vector<VkPhysicalDevice>>();

    return suitable_physical_devices;
}

}  // namespace vke
