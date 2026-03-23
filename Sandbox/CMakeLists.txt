set(PROJECT_NAME VulkanSandbox)

add_executable(${PROJECT_NAME}
    src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    VulkanRenderer
    glfw 
    Vulkan::Vulkan)