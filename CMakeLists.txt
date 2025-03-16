cmake_minimum_required(VERSION 3.28)
project(portfolio LANGUAGES C CXX)

find_program(GLSLC glslangValidator)
if(NOT GLSLC)
    message(FATAL_ERROR "glslangValidator not found!")
endif()

# Set C and C++ standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
# set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall)

include(FetchContent)

# Fetch SFML from GitHub.
FetchContent_Declare(
    Fetch_SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
)
FetchContent_MakeAvailable(Fetch_SFML)

FetchContent_Declare(
    Fetch_VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.2.1
)
FetchContent_MakeAvailable(Fetch_VulkanMemoryAllocator)

FetchContent_Declare(
    Fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
    GIT_TAG v1.4.310
)
FetchContent_MakeAvailable(Fetch_vk_bootstrap)

find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(cleanup_stack)
add_subdirectory(game_of_life)
add_subdirectory(glowing_dots)
add_subdirectory(tictactoe)
add_subdirectory(vk_test)
add_subdirectory(vk_test_v2)
