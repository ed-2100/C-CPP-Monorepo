
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

struct FrameData {
    VkCommandPool command_pool;
    VkCommandBuffer main_command_buffer;
    VkSemaphore swaphcain_semaphore, render_semaphore;
    VkFence render_fence;
};

constexpr uint32_t FRAME_OVERLAP = 2;

struct VulkanEngine {
    VulkanEngine();
    ~VulkanEngine();

    // Delete copy constructor and copy assignment operator
    VulkanEngine(const VulkanEngine &) = delete;
    VulkanEngine &operator=(const VulkanEngine &) = delete;

    // Delete move constructor and move assignment operator
    VulkanEngine(VulkanEngine &&) = delete;
    VulkanEngine &operator=(VulkanEngine &&) = delete;

    VkExtent2D window_extent = {500, 500};

    SDL_Window *window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkSurfaceKHR surface;

    VkPhysicalDevice physical_device;
    VkDevice device;

    // ----- Swapchain -----
    VkSwapchainKHR swapchain;
    VkFormat swapchain_image_format;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    VkExtent2D swapchain_extent;

    size_t frame_number = 0;
    FrameData frames[FRAME_OVERLAP];

    VkQueue graphics_queue;
    uint32_t graphics_queue_family;

public:
    void run();

private:
    void draw();

    void init_vulkan();

    void init_swapchain();
    void create_swapchain();
    void destroy_swapchain();

    void init_commands();
    void init_sync_structures();

    FrameData &get_current_frame();
};

// VulkanEngine(VulkanEngine &&rhs) noexcept : window(std::exchange(rhs.window, nullptr)) {};
// VulkanEngine &operator=(VulkanEngine &&rhs) noexcept {
//     if (this != &rhs) {
//         this->~VulkanEngine();
//         window = std::exchange(rhs.window, nullptr);
//     }
//     return *this;
// }
