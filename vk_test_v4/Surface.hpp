#include <vulkan/vulkan_core.h>

struct Surface {
  Surface() = delete;
  explicit Surface(VkSurfaceKHR surface) : surface(surface) {}
  virtual ~Surface() = 0;

  Surface(Surface const&) = delete;
  Surface& operator=(Surface const&) = delete;

  Surface(Surface const&&) = delete;
  Surface& operator=(Surface const&&) = delete;

  constexpr operator VkSurfaceKHR() const { return surface; }

  VkSurfaceKHR surface = VK_NULL_HANDLE;
};

inline Surface::~Surface() {}
