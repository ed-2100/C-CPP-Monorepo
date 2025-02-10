# vk_test_v4
The point of this one is to avoid using Vulkan RAII headers, as they balloon compilation times.

This is a failiure, because there is a simple solution to this problem.

Use PCH.