#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <vector>

namespace vke {

class InstanceInner {
    const VkInstance instance;
    const VkDebugUtilsMessengerEXT debug_messenger;
    const PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

    InstanceInner(const InstanceInner&) = delete;
    InstanceInner& operator=(const InstanceInner&) = delete;

    InstanceInner(InstanceInner&&) = delete;
    InstanceInner& operator=(InstanceInner&&) = delete;

public:
    InstanceInner() = delete;
    InstanceInner(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger);
    ~InstanceInner();

    inline operator VkInstance() const { return instance; }
};

class Instance {
    const std::shared_ptr<InstanceInner> inner;

public:
    Instance() = delete;
    inline Instance(std::shared_ptr<InstanceInner> inner) : inner(inner) { assert(inner); }

    inline operator VkInstance() const { return *inner; }
};

class InstanceBuilder {
    bool validation_layers;
    std::vector<const char*> layers;
    std::vector<const char*> extensions;

    const void* pNext = nullptr;

public:
    inline InstanceBuilder() {}

    Instance build();

    InstanceBuilder& with_validation_layers();
    InstanceBuilder& with_extensions(std::span<const char* const> extensions);
    InstanceBuilder& with_layers(std::span<const char* const> layers);
};

}  // namespace vke
