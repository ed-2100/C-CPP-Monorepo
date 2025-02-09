file(GLOB SOURCES "*.cpp")
get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${MODULE_NAME})
target_sources(${MODULE_NAME} PRIVATE ${SOURCES})
target_link_libraries(${MODULE_NAME} PRIVATE vulkan SDL3)
