set(PROJECT_NAME VulkanSandbox)

add_executable(${PROJECT_NAME}
    src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    VulkanRenderer
    MorphEngine
    glfw 
    Vulkan::Vulkan)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Modules/VulkanRenderer/shaders
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
)