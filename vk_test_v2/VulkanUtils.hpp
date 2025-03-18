#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <ranges>
#include <vector>

namespace vke {

inline void checkResult(VkResult result) {
    if (result) {
        throw std::runtime_error(
            std::format("Encountered vulkan error: {}", static_cast<int>(result))
        );
    }
}

// ----- Instance -----

class Instance {
    class Inner {
        const VkInstance instance;

        Inner(const Inner&) = delete;
        Inner& operator=(const Inner&) = delete;

        Inner(Inner&&) = delete;
        Inner& operator=(Inner&&) = delete;

    public:
        Inner() = delete;
        Inner(const VkInstanceCreateInfo& create_info);
        ~Inner();

        const PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        const PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

        inline operator VkInstance() const { return instance; }
    };

    const std::shared_ptr<Inner> inner;

public:
    Instance() = delete;
    inline Instance(const VkInstanceCreateInfo& create_info)
        : inner(std::make_shared<Inner>(create_info)) {}

    inline Inner* operator->() const { return &(*inner); }

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

// ----- DebugUtilsMessengerEXT -----

class DebugUtilsMessengerEXT {
    class Inner {
        const Instance instance;
        const VkDebugUtilsMessengerEXT debug_messenger;

    public:
        Inner() = delete;
        Inner(Instance instance, const VkDebugUtilsMessengerCreateInfoEXT& create_info);
        ~Inner();

        inline operator VkDebugUtilsMessengerEXT() const { return debug_messenger; }
    };

    const std::shared_ptr<Inner> inner;

public:
    DebugUtilsMessengerEXT() = delete;
    inline DebugUtilsMessengerEXT(
        Instance instance,
        const VkDebugUtilsMessengerCreateInfoEXT& create_info
    )
        : inner(std::make_shared<Inner>(instance, create_info)) {}

    inline operator VkDebugUtilsMessengerEXT() const { return *inner; }
};

// ----- PhysicalDeviceSelector -----

/// WARNING: This contains delicate code!
class PhysicalDeviceSelector {
    struct MemoryLayout {
        VkStructureType sType;
        void* pNext;
        VkBool32 first;
        VkBool32 second;
    };

    static constexpr size_t offset_sType = offsetof(MemoryLayout, sType);
    static constexpr size_t offset_pNext = offsetof(MemoryLayout, pNext);
    static constexpr size_t offset_first = offsetof(MemoryLayout, first);

    struct Node {
        std::byte* user;
        std::byte* api;
        size_t size;
        Node* next;

        Node(size_t size);
        ~Node();

        inline constexpr VkStructureType& api_sType() {
            return *(VkStructureType*)(api + offset_sType);
        }
        inline constexpr VkStructureType& user_sType() {
            return *(VkStructureType*)(user + offset_sType);
        }
        inline constexpr void*& api_pNext() { return *(void**)(api + offset_pNext); }
        inline constexpr void*& user_pNext() { return *(void**)(user + offset_pNext); }
        inline auto get_bool_pairs() const {
            return std::views::iota(size_t(0), (size - offset_first) / sizeof(VkBool32)) |
                   std::views::transform([&](size_t i) {
                       return std::tuple(
                           ((VkBool32*)(user + offset_first))[i],
                           ((VkBool32*)(api + offset_first))[i]
                       );
                   });
        }
    };

    Node* head = nullptr;
    VkPhysicalDeviceFeatures2 features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

    void with_features_struct_inner(std::byte* to_add, size_t size);
    bool is_suitable(VkPhysicalDevice physical_device) const;

public:
    ~PhysicalDeviceSelector();

    template <typename T>
    inline PhysicalDeviceSelector& with_features_struct(const T& to_add) {
        with_features_struct_inner((std::byte*)&to_add, sizeof(to_add));
        return *this;
    }

    template <>
    inline PhysicalDeviceSelector& with_features_struct(const VkPhysicalDeviceFeatures2& to_add) {
        memcpy((std::byte*)&features, &to_add, sizeof(features));
        return *this;
    }

    std::vector<VkPhysicalDevice> select(VkInstance instance) const;
};

}  // namespace vke
