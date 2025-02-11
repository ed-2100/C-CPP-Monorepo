#include <vulkan/vulkan.hpp>

struct Surface {
  Surface() = delete;
  explicit Surface(vk::SurfaceKHR surface) : surface(surface) {}
  virtual ~Surface() = 0;

  Surface(Surface const&) = delete;
  Surface& operator=(Surface const&) = delete;

  Surface(Surface&& rhs) noexcept : surface(std::exchange(rhs.surface, {})) {};
  Surface& operator=(Surface&& rhs) noexcept {
    if (this != &rhs) {
      std::swap(surface, rhs.surface);
    }
    return *this;
  }

  constexpr operator vk::SurfaceKHR() const { return surface; }

  vk::SurfaceKHR surface = VK_NULL_HANDLE;
};

inline Surface::~Surface() {}
