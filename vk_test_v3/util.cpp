#include "util.hpp"

vk::raii::SurfaceKHR Window::createSurface(vk::raii::Instance const& instance) {
  VkSurfaceKHR _surface;
  VkResult err = glfwCreateWindowSurface(
      static_cast<VkInstance>(*instance), handle, nullptr, &_surface);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window!");
  }
  std::cout << "@S" << std::endl;
  return vk::raii::SurfaceKHR(instance, _surface);
}

Window glfwContext::createWindow(std::string const& windowName,
                                 vk::Extent2D const& extent) const {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow* window = glfwCreateWindow(
      extent.width, extent.height, windowName.c_str(), nullptr, nullptr);

  std::cout << "@W" << std::endl;
  return Window(*this, window, windowName, extent);
}
