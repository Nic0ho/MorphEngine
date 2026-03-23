set(LIB_NAME VulkanRenderer)

add_library(VulkanRenderer STATIC
    src/core.cpp
    src/device.cpp
    src/util.cpp)

target_link_libraries(VulkanRenderer PRIVATE 
    Vulkan::Vulkan
    glfw)

target_include_directories(VulkanRenderer PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Engine/include)