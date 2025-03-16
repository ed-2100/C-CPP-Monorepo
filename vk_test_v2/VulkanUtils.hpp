#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <vector>

namespace vke {

class InstanceInner {
    const VkInstance instance;

    InstanceInner(const InstanceInner&) = delete;
    InstanceInner& operator=(const InstanceInner&) = delete;

    InstanceInner(InstanceInner&&) = delete;
    InstanceInner& operator=(InstanceInner&&) = delete;

public:
    InstanceInner() = delete;
    InstanceInner(const VkInstanceCreateInfo&);
    ~InstanceInner();

    const PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    const PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

    inline operator VkInstance() const { return instance; }
};

class Instance {
    const std::shared_ptr<InstanceInner> inner;

public:
    Instance() = delete;
    inline Instance(std::shared_ptr<InstanceInner> inner) : inner(inner) { assert(inner); }

    inline InstanceInner* operator->() const { return &(*inner); }

    inline operator VkInstance() const { return *inner; }
};

class InstanceBuilder {
    bool validation_layers;
    std::vector<const char*> layers;
    std::vector<const char*> extensions;

    void* pNext = nullptr;

public:
    inline InstanceBuilder() {}

    Instance build();

    InstanceBuilder& with_validation_layers();
    InstanceBuilder& with_extensions(std::span<const char* const> extensions);
    InstanceBuilder& with_layers(std::span<const char* const> layers);

    template <typename T>
    inline InstanceBuilder& push_pNext(T* to_push) {
        to_push->pNext = pNext;
        pNext = to_push;
        return *this;
    }
};

class DebugUtilsMessengerEXTInner {
    const Instance instance;
    const VkDebugUtilsMessengerEXT debug_messenger;

public:
    DebugUtilsMessengerEXTInner() = delete;
    DebugUtilsMessengerEXTInner(
        Instance instance,
        const VkDebugUtilsMessengerCreateInfoEXT& create_info
    );
    ~DebugUtilsMessengerEXTInner();

    inline operator VkDebugUtilsMessengerEXT() const { return debug_messenger; }
};

class DebugUtilsMessengerEXT {
    const std::shared_ptr<DebugUtilsMessengerEXTInner> inner;

public:
    DebugUtilsMessengerEXT() = delete;
    inline DebugUtilsMessengerEXT(
        Instance instance,
        const VkDebugUtilsMessengerCreateInfoEXT& create_info
    )
        : inner(std::make_shared<DebugUtilsMessengerEXTInner>(instance, create_info)) {}

    inline operator VkDebugUtilsMessengerEXT() const { return *inner; }
};

}  // namespace vke
