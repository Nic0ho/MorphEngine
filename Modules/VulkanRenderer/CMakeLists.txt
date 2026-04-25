set(LIB_NAME VulkanRenderer)

add_library(VulkanRenderer STATIC
    src/core.cpp
    src/device.cpp
    src/util.cpp
    src/queue.cpp
    src/wrapper.cpp
    src/shader.cpp
    src/graphics_pipeline.cpp
    src/glfw_vulkan.cpp
    src/stb_image.cpp)

if (WIN32)
    target_link_directories(VulkanRenderer PUBLIC
        $ENV{VULKAN_SDK}/Lib)

    target_link_libraries(VulkanRenderer PRIVATE 
        Vulkan::Vulkan
        glfw
        MorphEngine
        
        debug glslangd
        optimized glslang
        
        debug glslang-default-resource-limitsd
        optimized glslang-default-resource-limits
        
        debug SPIRVd
        optimized SPIRV
        
        debug SPIRV-Toolsd
        optimized SPIRV-Tools
        
        debug SPIRV-Tools-optd
        optimized SPIRV-Tools-opt
    )
else()
    target_link_libraries(VulkanRenderer PRIVATE
        Vulkan::Vulkan
        glfw
        MorphEngine
        glslang
        glslang-default-resource-limits
        SPIRV
        SPIRV-Tools
        SPIRV-Tools-opt)
endif()

target_include_directories(VulkanRenderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/SPIRV-Reflect
    ${Vulkan_INCLUDE_DIRS})

target_sources(VulkanRenderer PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SPIRV-Reflect/spirv_reflect.c)