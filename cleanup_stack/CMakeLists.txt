file(GLOB SOURCES "*.c")
get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${MODULE_NAME})
target_sources(${MODULE_NAME} PRIVATE ${SOURCES})
target_compile_options(${MODULE_NAME} PRIVATE -Wno-pragma-pack)
