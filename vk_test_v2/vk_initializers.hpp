#pragma once

#include <vulkan/vulkan.h>

namespace vkutil {

VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);

} // namespace vkutil
