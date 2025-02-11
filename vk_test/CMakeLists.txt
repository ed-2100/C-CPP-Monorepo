file(GLOB SHADER_FILES "*.vert" "*.frag" "*.comp")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPV_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${GLSLC} -V ${SHADER} -o ${SPV_FILE} --quiet
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
    )
    list(APPEND SPV_FILES ${SPV_FILE})
endforeach()

file(GLOB SOURCES "*.cpp")
get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SHADER_TARGET "${MODULE_NAME}_shaders")
add_custom_target(${SHADER_TARGET} DEPENDS ${SPV_FILES})

add_executable(${MODULE_NAME})
target_sources(${MODULE_NAME} PRIVATE ${SOURCES})
target_link_libraries(${MODULE_NAME} PRIVATE vulkan SDL3)
add_dependencies(${MODULE_NAME} ${SHADER_TARGET})
target_precompile_headers(${MODULE_NAME} PRIVATE vulkan_raii_pch.hpp)
