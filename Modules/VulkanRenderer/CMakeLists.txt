set(LIB_NAME VulkanRenderer)

add_library(VulkanRenderer STATIC
    src/core.cpp
    src/device.cpp
    src/util.cpp
    src/queue.cpp
    src/wrapper.cpp
    src/shader.cpp)

target_link_libraries(VulkanRenderer PRIVATE 
    Vulkan::Vulkan
    glfw
    MorphEngine
    glslang
    glslang-default-resource-limits
    SPIRV
    SPVRemapper)

target_include_directories(VulkanRenderer PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/SPIRV-Reflect)

target_sources(VulkanRenderer PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SPIRV-Reflect/spirv_reflect.c)