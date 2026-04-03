set(LIB_NAME VulkanRenderer)

add_library(VulkanRenderer STATIC
    src/core.cpp
    src/device.cpp
    src/util.cpp
    src/queue.cpp
    src/wrapper.cpp
    src/shader.cpp
    src/graphics_pipeline.cpp)

add_custom_command(TARGET VulkanRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        ${CMAKE_BINARY_DIR}/bin/shaders)

target_link_directories(VulkanRenderer PUBLIC
    $ENV{VULKAN_SDK}/Lib)

target_link_libraries(VulkanRenderer PRIVATE 
    Vulkan::Vulkan
    glfw
    MorphEngine
    glslangd
    glslang-default-resource-limitsd
    SPIRVd
    SPIRV-Toolsd
    SPIRV-Tools-optd)

target_include_directories(VulkanRenderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Engine/include8
    ${CMAKE_SOURCE_DIR}/ThirdParty/SPIRV-Reflect)

target_sources(VulkanRenderer PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SPIRV-Reflect/spirv_reflect.c)