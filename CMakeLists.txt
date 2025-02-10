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
add_compile_options(-Wall -Wno-pragma-pack)

# Dependencies
find_package(SFML COMPONENTS graphics REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)
# find_package(glfw3 REQUIRED)
# find_package(glm REQUIRED)

add_subdirectory(cleanup_stack)
add_subdirectory(game_of_life)
add_subdirectory(glowing_dots)
add_subdirectory(tictactoe)
add_subdirectory(vk_test_v1)
add_subdirectory(vk_test_v2)
add_subdirectory(vk_test_v3)
add_subdirectory(vk_test_v4)
